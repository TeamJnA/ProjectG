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

/*
* 
*/
void UAT_WaitForHoldInput::Activate()
{
	TimeHeld = 0.0f;
	bInputHeld = false;

	if (AbilitySystemComponent.IsValid())
	{
		// �Է� Ű(F)�� ���� ��ȭ ���� (����/������)
		InputTagEventHandle = AbilitySystemComponent->RegisterGameplayTagEvent(InteractInputTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAT_WaitForHoldInput::OnInteractInputChanged);
	}
}

/*
* Ȧ�� �ð� ������Ʈ
* Ȧ�� �ð��� ���� ����� ������Ʈ
* Ȧ�� �Ϸ�� �Ϸ� �˸�
*/
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

/*
* Ȧ�� ������ Ȥ�� �Ϸ� �������� Ȧ�� �ߴܿ� ���� Task ����
* Ȧ���� �ߴ��� �������� Ȧ���� �Ϸ�� ��� -> �Ϸ� �˸�
* ����� �ʱ�ȭ
*/
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

/*
* �Է� ó��
*/
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
