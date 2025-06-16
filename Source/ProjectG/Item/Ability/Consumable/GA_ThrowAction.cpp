// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_ThrowAction.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

/// <summary>
/// This abiltiy is called from GA_Item_Throw(and childs).
/// Do ThrowActionMonatage once, end ability and remove ability.
/// This ability is executed with GiveAbilityAndActivateOnce, so it is removed when it ends.
/// Through this ability, a tag is activated while playing an animation montage,
/// and it can be removed when the animation ends.
/// This works smoothly and behaves synchronously on both the server and the client.
/// </summary>
UGA_ThrowAction::UGA_ThrowAction()
{
	FGameplayTag ThrowActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	/*
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction")));
	SetAssetTags(TagContainer);
	*/

	ActivationOwnedTags.AddTag(ThrowActionActivateTag);

	ActivationBlockedTags.AddTag(ThrowActionActivateTag);

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ThrowAnimMontageRef(TEXT("/Game/ProjectG/Character/Animation/Throw/AM_Throw.AM_Throw"));
	if (ThrowAnimMontageRef.Object) {
		ThrowAnimMontage = ThrowAnimMontageRef.Object;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Anim montage not found in GA_ThrowAction."));
	}
}

void UGA_ThrowAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("ThrowAction activated."));

	// Play Throw anim montage
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			1.0f,
			TEXT("Throw")
		);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_ThrowAction::OnCompletedAnimMontage);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_ThrowAction::OnCompletedAnimMontage);
	MontageTask->ReadyForActivation();
}

void UGA_ThrowAction::OnCompletedAnimMontage()
{
	UE_LOG(LogTemp, Log, TEXT("ThrowAction completed."));

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}