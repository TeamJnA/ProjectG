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
	// This ability only run on server.
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	// Starts the ability task to trace forward from the given start location to detect objects in front.
	UCameraComponent* LinetraceStartPosition = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo())->GetFollowCamera();

	WaitForInteractionTarget = UAT_WaitForInteractionTarget::WaitForInteractionTarget(this, LinetraceStartPosition, true);
	WaitForInteractionTarget->InteractionTarget.AddDynamic(this, &UGA_Interact::WaitInteractionInput);
	WaitForInteractionTarget->ReadyForActivation();

}

// This ability activate automatically when granted. This is passive activate ability.
void UGA_Interact::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}

void UGA_Interact::WaitInteractionInput(AActor* TargetActor)
{
	// If there is no object in front, reset the stored target variable.
	// And end the ability task that was waiting for interaction input.
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Linetrace interact target is changed to null or invalid object."));
		AbilityToInteract = nullptr;
		if (WaitForInteractTag)
		{
			WaitForInteractTag->EndTask();
			WaitForInteractTag = nullptr;
		}
		return;
	}

	// Cache the target in a local variable to prevent changes during the asynchronous Ability Task.
	TWeakObjectPtr<AActor> TargetActorCopy;
	TargetActorCopy = TargetActor; 

	// If a previously running task exists, end it before creating a new one.
	// This prevents multiple tasks from being created.
	if (WaitForInteractTag)
	{
		WaitForInteractTag->EndTask();
		WaitForInteractTag = nullptr;
	}

	// Starts an Ability Task that waits for the Interact tag and triggers the interaction once the tag is received.
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
	UE_LOG(LogTemp, Log, TEXT("Entered InteractWithTarget with TargetActor."));

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetActor is Null in InteractWithTarget"));
		WaitForInteractTag->EndTask();
		WaitForInteractTag = nullptr;
		return;
	}

	// Get ability and give ability to character
	IInteractableActorInterface* InteractInterface = Cast<IInteractableActorInterface>(TargetActor);

	// Check the interface to ensure the target hasn't changed while waiting for the input tag.
	if (!InteractInterface)
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

	// To prevent the ability is granted multiple times, remove the existing instance and add a new one.
	if (ASC->FindAbilitySpecFromClass(AbilityToInteract))
	{
		FGameplayAbilitySpec* ClearSpec = ASC->FindAbilitySpecFromClass(AbilityToInteract);
		ASC->ClearAbility(ClearSpec->Handle);
		UE_LOG(LogTemp, Warning, TEXT("Ability already granted. Remove and re-grant."));
	}

	const FGameplayAbilitySpec AbilitySpec(AbilityToInteract, 1);
	FGameplayAbilitySpecHandle InteractAbilityHandle = ASC->GiveAbility(AbilitySpec);

	// Make GameplayEvent and activate ability with event data.
	AActor* Instigator = GetAvatarActorFromActorInfo();
	FGameplayTag InteractEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));

	FGameplayEventData Payload;
	Payload.EventTag = InteractEventTag;
	Payload.Instigator = Instigator;
	Payload.Target = TargetActor;

	FGameplayAbilityActorInfo ActorInfo = GetActorInfo();

	// EventData is const, so the Payload.Target cannot be modified directly.
	// Instead, We can caching target actor in the character. 
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
		// If Failed to interact with target, Restart from the initial task WaitForInteractionTarget.
		// Because the target actor is changed when linetrace find new object.
		// But on failure, the previous object remains and is do not pass the object pointer.

		ASC->ClearAbility(InteractAbilityHandle);
		UE_LOG(LogTemp, Warning, TEXT("Fail to trigger ability"));

		if (WaitForInteractionTarget)
		{
			WaitForInteractionTarget->EndTask();
			WaitForInteractionTarget = nullptr;
		}
		UCameraComponent* LinetraceStartPosition = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo())->GetFollowCamera();

		WaitForInteractionTarget = UAT_WaitForInteractionTarget::WaitForInteractionTarget(this, LinetraceStartPosition, true);
		WaitForInteractionTarget->InteractionTarget.AddDynamic(this, &UGA_Interact::WaitInteractionInput);
		WaitForInteractionTarget->ReadyForActivation();
	}

	WaitForInteractTag->EndTask();
	WaitForInteractTag = nullptr;
}