// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_Item_Throw.h"
#include "AbilitySystemComponent.h"

#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "Item/Ability/Consumable/GA_ThrowAction.h"

#include "Character/PGPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogAbility);

UGA_Item_Throw::UGA_Item_Throw()
{
	MouseLeftTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseLeft"));
	MouseRightTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseRight"));

	HandActionTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	bThrowReady = false;

	// This ability is activated in server.
	// If activated with local predict, there is an issue with spawn projectile item.
	// NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ThrowAnimMontageRef(TEXT("/Game/ProjectG/Character/Animation/Throw/AM_Throw.AM_Throw"));
	if (ThrowAnimMontageRef.Object){
		ThrowAnimMontage = ThrowAnimMontageRef.Object;
	}
	else{
		UE_LOG(LogAbility, Warning, TEXT("Anim montage not found in GA_Item_Throw."));
	}
}

void UGA_Item_Throw::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogAbility, Log, TEXT("%s | %s ability activated."), *GetClass()->GetName(), *GetName());
	bThrowReady = false;

	// Wait to input mouse left or mouse right button.
	// Right : Draw throwable item trajactory.
	// Left : Throw item.

	UAT_PGWaitGameplayTagReAdded* WaitForRightMouseTag = UAT_PGWaitGameplayTagReAdded::WaitGameplayTagReAdded(
		this, 
		MouseRightTag);
	WaitForRightMouseTag->Added.AddDynamic(this, &UGA_Item_Throw::MouseRight);
	WaitForRightMouseTag->ReadyForActivation();
	
	UAT_PGWaitGameplayTagReAdded* WaitForLeftMouseTag = UAT_PGWaitGameplayTagReAdded::WaitGameplayTagReAdded(
		this,
		MouseLeftTag);
	WaitForLeftMouseTag->Added.AddDynamic(this, &UGA_Item_Throw::MouseLeft);
	WaitForLeftMouseTag->ReadyForActivation();
}

void UGA_Item_Throw::MouseLeft()
{
	UE_LOG(LogAbility, Log, TEXT("MouseLeft Input. Start to throw item! %s | %s"), *GetClass()->GetName(), *GetName());

	// Throwing Start. Check HandAction.Lock tag.
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionTag))
	{
		UE_LOG(LogAbility, Log, TEXT("Cannot do %s during hand action."), *GetName());
		return;
	}
		
	// Spawn item actor only on the server.
	// ThrowAnimMontageAbility is local predicted. It is also run on th server. This manage hand action tag.
	// And end this ability by remove ability's owner item from inventory.
	if (HasAuthority(&CurrentActivationInfo))
	{
		SpawnProjectileActor();

		FGameplayAbilitySpec AbilitySpec(UGA_ThrowAction::StaticClass(), 1);
		AbilitySystemComponent->GiveAbilityAndActivateOnce(AbilitySpec);

		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
		if (!PGPC) {
			UE_LOG(LogAbility, Warning, TEXT("Cannot found avatar actor in RemoveItem %s"), *GetName());
			return;
		}
		PGPC->RemoveItemFromInventory();
	}
}

void UGA_Item_Throw::MouseRight()
{
	//Cannot do ThrowReady while HandAction.
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s ::RightInputCanceled"), *GetName());
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionTag))
	{
		UE_LOG(LogAbility, Log, TEXT("Cannot do %s ::RightInputCanceled during hand action."), *GetName());
		return;
	}

	UE_LOG(LogAbility, Log, TEXT("MouseRight Input. Ready to throw. %s | %s"), *GetClass()->GetName(), *GetName());
	bThrowReady = true;
	
	//Wait at throw ready
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			0.0f,
			TEXT("Throw")
		);

	//Do not into RightInputCanceled by setting RightClickCanceled.
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Item_Throw::ThrowReadyCanceled);
	MontageTask->ReadyForActivation();

	///
	/// Draw throw prediction line.
	/// 
	/// 
	/// 
	/// 
	/// 
	/// 
	/// 
	/// 

	// Wait for tag removed : Right Input.
	// Play anim montage to return hand to default position.
	UAbilityTask_WaitGameplayTagRemoved* TagRemovedTask =
		UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
			this,
			MouseRightTag
		);
	TagRemovedTask->Removed.AddDynamic(this, &UGA_Item_Throw::RightInputCanceled);
	TagRemovedTask->ReadyForActivation();
}

void UGA_Item_Throw::RightInputCanceled()
{
	if (bThrowReady == false)
		return;
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s ::RightInputCanceled"), *GetName());
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionTag))
	{
		UE_LOG(LogAbility, Log, TEXT("Cannot do %s ::RightInputCanceled during hand action."), *GetName());
		return;
	}

	// Play anim montage to return hand to default position.
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			-1.0f,
			TEXT("ThrowStart")
		);
	MontageTask->ReadyForActivation();
}

// This fuction implemented in child classes.
// It is implemented differently depending on which actor item to spawn.
void UGA_Item_Throw::SpawnProjectileActor()
{
	UE_LOG(LogAbility, Log, TEXT("SpawnProjectileActor"));
}

void UGA_Item_Throw::ThrowReadyCanceled()
{
	bThrowReady = false;
}

void UGA_Item_Throw::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}