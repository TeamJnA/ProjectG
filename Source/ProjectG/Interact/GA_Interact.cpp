// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/GA_Interact.h"

#include "Interact/Task/AT_WaitForInteractionTarget.h"
#include "Interact/Task/AT_PGWaitGameplayTagAdded.h"
#include "Interact/Task/AT_WaitForHoldInput.h"

#include "Character/PGPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Character/Component/PGInventoryComponent.h"
#include "Camera/CameraComponent.h"

#include "Interface/InteractableActorInterface.h"

#include "Item/PGItemActor.h"
#include "Level/PGDoor1.h"
#include "Level/PGExitDoor.h"


/*
* ���� Ʈ���̽� �½�ũ�� Ȱ��ȭ�Ͽ� �÷��̾� ĳ���� ī�޶� ���� Ž��
*/
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
	APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter || !PlayerCharacter->GetFirstPersonCamera())
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::ActivateAbility: No valid character or camera"));
		return;
	}
	UCameraComponent* LinetraceStartPosition = PlayerCharacter->GetFirstPersonCamera();

	if (UPGInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent())
	{
		Inventory->OnCurrentSlotIndexChanged.AddDynamic(this, &UGA_Interact::OnInventorySlotChanged);
	}

	WaitForInteractionTarget = UAT_WaitForInteractionTarget::WaitForInteractionTarget(this, LinetraceStartPosition, true);
	WaitForInteractionTarget->InteractionTarget.AddDynamic(this, &UGA_Interact::WaitInteractionInput);
	WaitForInteractionTarget->ReadyForActivation();
}

void UGA_Interact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// �����Ƽ�� ����Ǹ� InventoryComponent delegate bind ����
	if (APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UPGInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent())
		{
			Inventory->OnCurrentSlotIndexChanged.RemoveDynamic(this, &UGA_Interact::OnInventorySlotChanged);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// This ability activate automatically when granted. This is passive activate ability.
void UGA_Interact::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
}

/*
* ���� �ִ� ��� ���� ó��
*/
void UGA_Interact::WaitInteractionInput(AActor* TargetActor)
{
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::WaitInteractionInput: Failed to cast AvatarActor to APGPlayerCharacter"));
		return;
	}

	OwnerCharacter->Client_PlayerStareAtTarget(TargetActor);

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::WaitInteractionInput: No valid ASC"));
		return;
	}
	// �÷��̾� �ִϸ��̼� ���� ���� ��� �Է� ����/����
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		if (WaitForHoldInputTask)
		{
			WaitForHoldInputTask->EndTask();
			WaitForHoldInputTask = nullptr;
		}
		if (WaitForInteractTag)
		{
			WaitForInteractTag->EndTask();
			WaitForInteractTag = nullptr;
		}
		return;
	}
	// ���� ���� �ִ� Ÿ���� ������ �ٸ� ��� -> ���� Ÿ�ٿ� ���� ��� Task ����
	if (TargetActor != CachedTargetActor.Get())
	{
		if (WaitForHoldInputTask)
		{
			WaitForHoldInputTask->EndTask();
			WaitForHoldInputTask = nullptr;
		}
		if (WaitForInteractTag)
		{
			WaitForInteractTag->EndTask();
			WaitForInteractTag = nullptr;
		}

		CachedTargetActor = TargetActor;
	}

	// ���� Ÿ���� ���� Ÿ�ٰ� ���� ���
	// Ÿ���� ���ų�, �̹� � ��ȣ�ۿ� Task�� ���� ���̸� ���ο� Tasks ����/���� x
	if (!TargetActor || WaitForHoldInputTask || WaitForInteractTag)
	{
		return;
	}

	/*
	* Ÿ�ٿ� ���� ��ȣ�ۿ� Task �б�/����
	* Ÿ���� ��ȣ�ۿ� ������ ���
	*	Ÿ���� ��ȣ�ۿ� Ÿ���� Instant�� ���(�Է� ��� ��ȣ�ۿ�) 
	*	-> WaitGameplayTagAddWithTarget ���, ��ȣ�ۿ� �õ� ��� ��ȣ�ۿ� ����
	*	Ÿ���� ��ȣ�ۿ� Ÿ���� Hold�� ���(�Է� Ȧ�带 ���� ��ȣ�ۿ�) 
	*	-> WaitForHoldInput ���, ��ȣ�ۿ� �õ� �� Ȧ�� ����� ������Ʈ, Ȧ�� �Ϸ� �� ��ȣ�ۿ� ����
	* Ÿ���� ��ȣ�ۿ� �Ұ����� ���(����� ���̰� ��� �����ε� ���踦 ������� ����) 
	* -> WaitGameplayTagAddWithTarget ���, ��ȣ�ۿ� �õ� ��� ���� �޽��� ���÷���
	*/
	if (IInteractableActorInterface* InteractableInterface = Cast<IInteractableActorInterface>(TargetActor))
	{
		FText FailureMesage;
		const bool bCanStartInteraction = InteractableInterface->CanStartInteraction(OwnerCharacter->GetAbilitySystemComponent(), FailureMesage);
		if (bCanStartInteraction)
		{
			const FInteractionInfo Info = InteractableInterface->GetInteractionInfo();
			switch (Info.InteractionType)
			{
				case EInteractionType::Instant:
				{
					FGameplayTagContainer InteractTagContainer = OwnerCharacter->GetInteractTag();
					if (InteractTagContainer.Num() > 0)
					{
						WaitForInteractTag = UAT_PGWaitGameplayTagAdded::WaitGameplayTagAddWithTarget(this, InteractTagContainer.First(), TargetActor, nullptr, true);
						WaitForInteractTag->Added.AddDynamic(this, &UGA_Interact::InteractWithTarget);
						WaitForInteractTag->ReadyForActivation();
					}
					break;
				}
				case EInteractionType::Hold:
				{
					WaitForHoldInputTask = UAT_WaitForHoldInput::WaitForHoldInput(this, Info.HoldDuration);
					WaitForHoldInputTask->OnHoldInputProgressUpdated.AddDynamic(this, &UGA_Interact::UpdateInteractionUI);
					WaitForHoldInputTask->OnHoldInputCompleted.AddDynamic(this, &UGA_Interact::OnHoldInputCompleted);
					WaitForHoldInputTask->OnHoldInputCancelled.AddDynamic(this, &UGA_Interact::OnHoldInputCancelled);
					WaitForHoldInputTask->ReadyForActivation();
					break;
				}
			}
		}
		else
		{
			FGameplayTagContainer InteractTagContainer = OwnerCharacter->GetInteractTag();
			if (InteractTagContainer.Num() > 0)
			{
				WaitForInteractTag = UAT_PGWaitGameplayTagAdded::WaitGameplayTagAddWithTarget(this, InteractTagContainer.First(), TargetActor, nullptr, true);
				WaitForInteractTag->Added.AddDynamic(this, &UGA_Interact::HandleFailedInteractionAttempt);
				WaitForInteractTag->ReadyForActivation();
			}
		}
	}	
}

void UGA_Interact::InteractWithTarget(AActor* TargetActor)
{
	UE_LOG(LogTemp, Log, TEXT("GA_Interact::InteractWithTarget: Entered InteractWithTarget with TargetActor."));

	// ���� Task pointer�� �����ִ� ��츦 ����� nullptr�� ����
	if (WaitForInteractTag) WaitForInteractTag = nullptr;
	if (WaitForHoldInputTask) WaitForHoldInputTask = nullptr;

	// ------------ ��ȿ�� �˻� --------------
	if (!TargetActor) 
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::InteractWithTarget: TargetActor is Null"));
		return;
	}
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::InteractWithTarget: Character is not valid"));
		return;
	}
	IInteractableActorInterface* InteractInterface = Cast<IInteractableActorInterface>(TargetActor);
	if (!InteractInterface) 
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::InteractWithTarget: TargetActor is not interactable."));
		return;
	}
	AbilityToInteract = InteractInterface->GetAbilityToInteract();
	if (!AbilityToInteract)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::InteractWithTarget: Cannot find Ability to interact"));
		return;
	}	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) 
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::InteractWithTarget: Cannot find ASC"));
		return;
	}
	// ------------ ��ȿ�� �˻� --------------

	// ���� �������� HandAction ���̸� return
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		return;
	}

	// ���� �������� Interact ��������(Target�� ��� ���� ��� �÷��̾ ���踦 ��� �ִ� ��������) ��˻�
	FText Failuremessage;
	if (!InteractInterface->CanStartInteraction(ASC, Failuremessage))
	{
		OwnerCharacter->Client_DisplayInteractionFailedMessage(Failuremessage);
		return;
	}

	// ----------- �����Ƽ ���� ���� ----------
	// �����Ƽ �ο� �� ����
	FGameplayAbilitySpecHandle InteractAbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityToInteract, 1));
	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.Target = TargetActor;

	OwnerCharacter->CacheInteractionTarget(TargetActor);

	FGameplayAbilityActorInfo ActorInfo = *GetCurrentActorInfo();
	const bool bSuccess = ASC->TriggerAbilityFromGameplayEvent(InteractAbilityHandle, &ActorInfo, Payload.EventTag, &Payload, *ASC);

	// ���н� ��ȸ�� �����Ƽ ����
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Interact: TriggerAbilityFromGameplayEvent failed for ability %s."), *AbilityToInteract->GetName());

		ASC->ClearAbility(InteractAbilityHandle);
	}
}

/*
* ���� �޽��� ���÷���
*/
void UGA_Interact::HandleFailedInteractionAttempt(AActor* TargetActor)
{
	if (WaitForInteractTag)
	{
		WaitForInteractTag = nullptr;
	}

	// ---- valid check ----
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::HandleFailedInteractionAttempt: no valid target"));
		return;
	}
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::HandleFailedInteractionAttempt: no valid character"));
		return;
	}
	IInteractableActorInterface* InteractableInterface = Cast<IInteractableActorInterface>(TargetActor);
	if (!InteractableInterface)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::HandleFailedInteractionAttempt: no valid interface"));
		return;
	}
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::HandleFailedInteractionAttempt: no valid ASC"));
		return;
	}
	// ---- valid check ----

	// ���� �������� HandAction ���̸� return
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		return;
	}

	FText FailureMessage;
	InteractableInterface->CanStartInteraction(OwnerCharacter->GetAbilitySystemComponent(), FailureMessage);
	OwnerCharacter->Client_DisplayInteractionFailedMessage(FailureMessage);
}

/*
* Ȧ�� ����� ������Ʈ
* Ȧ�� ����� ���÷��� ���� ������Ʈ
*/
void UGA_Interact::UpdateInteractionUI(float Progress)
{
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (OwnerCharacter)
	{
		// Update progress bar
		OwnerCharacter->Client_UpdateInteractionProgress(Progress);
	}
}

/*
* Ȧ�� �Ϸ� �� ��ȣ�ۿ� ����
*/
void UGA_Interact::OnHoldInputCompleted()
{
	InteractWithTarget(CachedTargetActor.Get());
}

/*
* Ȧ�� �ߴ� �� �ʱ�ȭ
*/
void UGA_Interact::OnHoldInputCancelled()
{
	WaitForHoldInputTask = nullptr;
}

/*
* ���� ��� �ٶ󺸴� ���¸� �����ϸ� ���踦 �� ���¿��� Inventory Slot�� �ٲپ��� �� 
* �ƹ��͵� ���� �ʾƵ� ������ ���踦 �� ���°� ���� Interaction�� ������ ������ �Ǵ°��� ���� 
* ���������� �ƹ��͵� ���� ���� ���¿��� ���� Inventory Slot���� �ٲپ��� ��
* �ƹ��͵� ���� ���� ���°� ���� Interaction�� �Ұ����� ������ �Ǵ°��� ����
*/
void UGA_Interact::OnInventorySlotChanged(int32 NewSlotIndex)
{
	// Inventory slot�� �ٲ�� ���� Task �ʱ�ȭ
	UE_LOG(LogTemp, Warning, TEXT("GA_Interact::OnInventorySlotChanged: Inventory changed while looking at same target, interaction condition changed"));
	if (WaitForHoldInputTask)
	{
		WaitForHoldInputTask->EndTask();
		WaitForHoldInputTask = nullptr;
	}
	if (WaitForInteractTag)
	{
		WaitForInteractTag->EndTask();
		WaitForInteractTag = nullptr;
	}
}
