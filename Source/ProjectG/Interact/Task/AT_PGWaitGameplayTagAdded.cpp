// Fill out your copyright notice in the Description page of Project Settings.

#include "AT_PGWaitGameplayTagAdded.h"

UAT_PGWaitGameplayTagAdded::UAT_PGWaitGameplayTagAdded(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAT_PGWaitGameplayTagAdded* UAT_PGWaitGameplayTagAdded::WaitGameplayTagAddWithTarget(
	UGameplayAbility* OwningAbility,
	FGameplayTag Tag, 
	AActor* TargetActor, 
	AActor* InOptionalExternalTarget, 
	bool OnlyTriggerOnce)
{
	UAT_PGWaitGameplayTagAdded* MyObj = NewAbilityTask<UAT_PGWaitGameplayTagAdded>(OwningAbility);
	MyObj->Tag = Tag;
	MyObj->TargetActor = TargetActor;
	MyObj->SetExternalTarget(InOptionalExternalTarget);
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;
	return MyObj;
}

void UAT_PGWaitGameplayTagAdded::Activate()
{
	//Bind tag num changed with GameplayTagCallback in UAbilityTask_WaitGameplayTag
	Super::Activate();
}

void UAT_PGWaitGameplayTagAdded::GameplayTagCallback(const FGameplayTag InTag, int32 NewCount)
{
	if (NewCount == 1)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Added.Broadcast(TargetActor.Get());
		}
		if (OnlyTriggerOnce)
		{
			EndTask();
		}
	}
}
