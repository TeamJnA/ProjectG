// Fill out your copyright notice in the Description page of Project Settings.

#include "Interact/Task/AT_WaitForHoldInput.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UAT_WaitForHoldInput* UAT_WaitForHoldInput::WaitForHoldInput(UGameplayAbility* OwningAbility, float HoldTime)
{
	UAT_WaitForHoldInput* MyObj = NewAbilityTask<UAT_WaitForHoldInput>(OwningAbility);
	MyObj->HoldDuration = HoldTime;
	MyObj->InteractInputTag = FGameplayTag::RequestGameplayTag(FName("Input.Interact"));
	MyObj->bTickingTask = true;

	return MyObj;
}

void UAT_WaitForHoldInput::Activate()
{
	TimeHeld = 0.0f;
	bInputHeld = false;

	if (AbilitySystemComponent.IsValid())
	{
		// 입력 키(F)의 상태 변화 감지 (눌림/떼어짐)
		InputTagEventHandle = AbilitySystemComponent->RegisterGameplayTagEvent(InteractInputTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAT_WaitForHoldInput::OnInteractInputChanged);
	}
}

void UAT_WaitForHoldInput::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bInputHeld)
	{
		TimeHeld += DeltaTime;
		float Progress = FMath::Clamp(TimeHeld / HoldDuration, 0.0f, 1.0f);

		// broadcast hold input progress
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnHoldInputProgressUpdated.Broadcast(Progress);
		}

		// broadcast hold input complete
		if (TimeHeld >= HoldDuration)
		{
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnHoldInputCompleted.Broadcast();
			}
			EndTask();
		}
	}
}

void UAT_WaitForHoldInput::OnDestroy(bool AbilityIsEnding)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (!bInputHeld && TimeHeld < HoldDuration)
		{
			OnHoldInputCancelled.Broadcast();
		}

		OnHoldInputProgressUpdated.Broadcast(0.0f);
	}

	if (AbilitySystemComponent.IsValid() && InputTagEventHandle.IsValid())
	{
		AbilitySystemComponent->UnregisterGameplayTagEvent(InputTagEventHandle, InteractInputTag, EGameplayTagEventType::NewOrRemoved);
	}

	Super::OnDestroy(AbilityIsEnding);
}

void UAT_WaitForHoldInput::OnInteractInputChanged(const FGameplayTag GameplayTag, int32 NewCount)
{
	if (GameplayTag == InteractInputTag)
	{
		// if key is pressed
		if (NewCount > 0)
		{
			bInputHeld = true;
			TimeHeld = 0.0f;
		}
		// if key is released
		else
		{
			bInputHeld = false;
			EndTask();
		}
	}
}
