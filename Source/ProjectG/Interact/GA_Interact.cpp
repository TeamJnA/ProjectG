// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/GA_Interact.h"
#include "Interact/Task/AT_WaitForInteractionTarget.h"

#include "Interact/Task/AT_PGWaitGameplayTagAdded.h"

#include "Character/PGPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "Interface/InteractableActorInterface.h"


void UGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}
	UCameraComponent* ActorCamera = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo())->GetFollowCamera();

	UAT_WaitForInteractionTarget* WaitForInteractionTarget = UAT_WaitForInteractionTarget::WaitForInteractionTarget(this, ActorCamera, true);
	WaitForInteractionTarget->InteractionTarget.AddDynamic(this, &UGA_Interact::WaitInteractionInput);
	WaitForInteractionTarget->ReadyForActivation();

}

//Activate ability automatically
void UGA_Interact::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}

void UGA_Interact::WaitInteractionInput(AActor* TargetActor)
{
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Target is Null"));
		AbilityToInteract = nullptr;
		if (WaitForInteractTag)
		{
			WaitForInteractTag->EndTask();
			WaitForInteractTag = nullptr;
		}
		return;
	}

	//Copy variable to local to avoid value changing during async task
	TWeakObjectPtr<AActor> TargetActorCopy;
	TargetActorCopy = TargetActor; 

	// If a previously running task exists, end it before creating a new one.
	// This prevents multiple tasks from being created.
	if (WaitForInteractTag)
	{
		WaitForInteractTag->EndTask();
		WaitForInteractTag = nullptr;
	}

	//Get interact tag and Wait for interact tag to activate. 
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter){
		UE_LOG(LogTemp, Error, TEXT("Failed to cast AvatarActor to APGPlayerCharacter"));
		return;
	}
	FGameplayTagContainer InteractTagContainer = OwnerCharacter->GetInteractTag();
	if (InteractTagContainer.Num() == 0){
		UE_LOG(LogTemp, Warning, TEXT("InteractTag is Null"));
		return;
	}

	FGameplayTag InteractTag = InteractTagContainer.First();

	WaitForInteractTag = UAT_PGWaitGameplayTagAdded::WaitGameplayTagAddWithTarget(
		this,
		InteractTag,
		TargetActorCopy.Get(),
		nullptr
	);
	WaitForInteractTag->Added.AddDynamic(this, &UGA_Interact::InteractWithTarget);
	WaitForInteractTag->ReadyForActivation();
}

void UGA_Interact::InteractWithTarget(AActor* TargetActor)
{

	UE_LOG(LogTemp, Log, TEXT("Get Interaction input and interact with target."));

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetActor is Null in InteractWithTarget"));
		WaitForInteractTag->EndTask();
		WaitForInteractTag = nullptr;
		return;
	}

	//Get ability and give ability to character
	IInteractableActorInterface* InteractInterface = Cast<IInteractableActorInterface>(TargetActor);
	if (!InteractInterface) //TargetActor has changed to null while wait for interact tag input
	{
		WaitForInteractTag->EndTask();
		WaitForInteractTag = nullptr;
		return;
	}
	AbilityToInteract = InteractInterface->GetAbilityToInteract();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot found ASC in GA_Interact::InteractWithTarget"));
		return;
	}

	//// In case the ability is granted multiple times, remove the existing instance and add a new one.
	if (ASC->FindAbilitySpecFromClass(AbilityToInteract))
	{
		FGameplayAbilitySpec* ClearSpec = ASC->FindAbilitySpecFromClass(AbilityToInteract);
		ASC->ClearAbility(ClearSpec->Handle);
		UE_LOG(LogTemp, Warning, TEXT("Ability already granted. Remove and re-grant."));
	}

	const FGameplayAbilitySpec AbilitySpec(AbilityToInteract, 1);
	FGameplayAbilitySpecHandle InteractAbilityHandle = ASC->GiveAbility(AbilitySpec);

	//Make GameplayEvent and activate ability with event
	AActor* Instigator = GetAvatarActorFromActorInfo();
	FGameplayTag InteractEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));

	FGameplayEventData Payload;
	Payload.EventTag = InteractEventTag;
	Payload.Instigator = Instigator;
	Payload.Target = TargetActor;

	FGameplayAbilityActorInfo ActorInfo = GetActorInfo();

	//EventData is const, so the Target cannot be modified directly.
	//Instead, We can caching target actor in the character. 
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter) {
		UE_LOG(LogTemp, Error, TEXT("Failed to cast AvatarActor to APGPlayerCharacter"));
		return;
	}
	
	OwnerCharacter->CacheInteractionTarget(TargetActor);

	const bool bSuccess = ASC->TriggerAbilityFromGameplayEvent(
		InteractAbilityHandle,
		&ActorInfo,
		InteractEventTag,
		&Payload,
		*ASC
	);
	
	if (!bSuccess)
	{
		ASC->ClearAbility(InteractAbilityHandle);
		UE_LOG(LogTemp, Log, TEXT("Fail to trigger ability"));
	}

	WaitForInteractTag->EndTask();
	WaitForInteractTag = nullptr;
}