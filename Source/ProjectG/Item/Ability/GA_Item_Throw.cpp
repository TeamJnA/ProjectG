// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_Item_Throw.h"
#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_Item_Throw::UGA_Item_Throw()
{
	MouseLeftTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseLeft"));
	MouseRightTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseRight"));

	bThrowReady = false;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ThrowAnimMontageRef(TEXT("/Game/ProjectG/Character/Animation/Throw/AM_Throw.AM_Throw"));
	if (ThrowAnimMontageRef.Object)
	{
		ThrowAnimMontage = ThrowAnimMontageRef.Object;
	}
}

void UGA_Item_Throw::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("UGA_Item_Throw ability activated."));

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
	
	//ThrowStart ThrowReady Throw
}

void UGA_Item_Throw::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Item_Throw::MouseLeft()
{
	UE_LOG(LogTemp, Log, TEXT("MouseLeft Input"));
	//Throwing Start. HandAction.Lock
	//But two cases
	//1. Start from Throw
	//2. Start from ThrowStart

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			1.0f,
			TEXT("ThrowStart")
		);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Item_Throw::OnCompletedFullThrowAnim);
	MontageTask->ReadyForActivation();
}

void UGA_Item_Throw::MouseRight()
{
	UE_LOG(LogTemp, Log, TEXT("MouseRight Input"));
	bThrowReady = true;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			0.0f,
			TEXT("Throw")
		);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Item_Throw::OnCompletedFullThrowAnim);
	MontageTask->ReadyForActivation();
}

void UGA_Item_Throw::OnCompletedFullThrowAnim()
{
	UE_LOG(LogTemp, Log, TEXT("Throw Completed"));
}

void UGA_Item_Throw::OnCompletedThrowStartAnim()
{
	UE_LOG(LogTemp, Log, TEXT("ThrowStart Completed"));
}
