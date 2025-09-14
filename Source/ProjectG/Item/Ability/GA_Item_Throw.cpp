// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_Item_Throw.h"

#include "AbilitySystemComponent.h"
#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Item/Ability/Consumable/GA_ThrowAction.h"
#include "Character/PGPlayerCharacter.h"
#include "PGLogChannels.h"

UGA_Item_Throw::UGA_Item_Throw()
{
	MouseLeftTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseLeft"));
	MouseRightTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseRight"));

	HandActionTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	bThrowReady = false;

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

	UE_LOG(LogAbility, Log, TEXT("MouseLeft Input. Start to throw item! %s | %s"), *GetClass()->GetName(), *GetName());
		
	// Spawn item actor on the server.
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
	// Cannot do ThrowReady while HandAction.
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
	
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			0.0f,
			TEXT("Throw")
		);

	// When other anim montage cancel the FullThrowAnimMontage, set bThrowReady true to do not play RightInputCanceled anim.
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Item_Throw::ThrowReadyCanceled);
	MontageTask->ReadyForActivation();

	/// TODO : Draw throw prediction line.
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
	// RightInputCanceled already canceled by LeftClick(Throwing) or other hand actions.
	if (bThrowReady == false)
	{
		return;
	}
	
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

void UGA_Item_Throw::ThrowReadyCanceled()
{
	bThrowReady = false;
}

void UGA_Item_Throw::SpawnProjectileActor()
{
}