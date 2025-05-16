// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"

UAT_PGWaitGameplayTagReAdded* UAT_PGWaitGameplayTagReAdded::WaitGameplayTagReAdded(UGameplayAbility* OwningAbility, 
	FGameplayTag Tag, 
	AActor* InOptionalExternalTarget,
	bool OnlyTriggerOnce)
{
	UAT_PGWaitGameplayTagReAdded* MyObj = NewAbilityTask<UAT_PGWaitGameplayTagReAdded>(OwningAbility);
	MyObj->Tag = Tag;
	MyObj->SetExternalTarget(InOptionalExternalTarget);
	MyObj->OnlyTriggerOnce = OnlyTriggerOnce;

	return MyObj;
}

void UAT_PGWaitGameplayTagReAdded::Activate()
{
	//Bind tag num changed with GameplayTagCallback in UAbilityTask_WaitGameplayTag
	Super::Activate();
}

void UAT_PGWaitGameplayTagReAdded::GameplayTagCallback(const FGameplayTag InTag, int32 NewCount)
{
	if (NewCount == 1)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Added.Broadcast();
		}
		if (OnlyTriggerOnce)
		{
			EndTask();
		}
	}
}
