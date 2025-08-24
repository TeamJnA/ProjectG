// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/GA_Interact.h"

#include "Interact/Task/AT_WaitForInteractionTarget.h"
#include "Interact/Task/AT_PGWaitGameplayTagAdded.h"
#include "Interact/Task/AT_WaitForHoldInput.h"

#include "Character/PGPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "Interface/InteractableActorInterface.h"

#include "Item/PGItemActor.h"
#include "Level/PGDoor1.h"
#include "Level/PGExitDoor.h"
#include "Character/Component/PGInventoryComponent.h"

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
	// 어빌리티가 종료되면 InventoryComponent delegate bind 해제
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

void UGA_Interact::WaitInteractionInput(AActor* TargetActor)
{
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::WaitInteractionInput: Failed to cast AvatarActor to APGPlayerCharacter"));
		return;
	}

	// UI(하이라이트, 메시지)는 항상 업데이트
	OwnerCharacter->Client_PlayerStareAtTarget(TargetActor);

	// HandAction이 진행 중이라면 상호작용 중단
	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_Interact::WaitInteractionInput: No valide ASC"));
		return;
	}
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		// 진행 중일 수 있는 모든 Task 정리
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

	// 현재 보고 있는 타겟이 이전과 다른 경우 -> 이전 타겟에 대한 모든 Task 정리
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

		// 이론상 필요없는데 나중에 버그 생기면 주석 제거
		//OwnerCharacter->Client_UpdateInteractionProgress(0.0f);
		CachedTargetActor = TargetActor;
	}

	// 현재 타겟이 이전 타겟과 같은 경우
	// 타겟이 없거나, 이미 어떤 상호작용 Task가 실행 중이면 새로운 Tasks 생성/제거 x
	if (!TargetActor || WaitForHoldInputTask || WaitForInteractTag)
	{
		// for debug
		//UE_LOG(LogTemp, Warning, TEXT("GA_Interact::WaitInteractionInput: Task is already running"))
		return;
	}

	// 타겟에 따라 상호작용 Task 분기, 진행
	if (IInteractableActorInterface* InteractableInterface = Cast<IInteractableActorInterface>(TargetActor))
	{
		// 상호작용 불가능한 경우 종료(문이 잠겨있고, 캐릭터가 열쇠를 들지 않은 경우)
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
		// 상호작용이 불가능한 경우(문이 잠겼는데, 열쇠를 들고있지 않음)
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

	// 이전 Task pointer가 남아있는 경우를 대비해 nullptr로 보장
	// Gemini피셜) Task의 경우 게임플레이 로직이 단일 스레드 진행이라 100% 종료되어있는 상태 -> 이 상황에서 EndTask 직접 호출은 오히려 위험하다고함
	if (WaitForInteractTag) WaitForInteractTag = nullptr;
	if (WaitForHoldInputTask) WaitForHoldInputTask = nullptr;

	// ------------ 유효성 검사 --------------
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
	// ------------ 유효성 검사 --------------

	// 현재 시점에서 HandAction 중이면 return
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		return;
	}

	// 현재 시점에서 Interact 가능한지(Target이 잠긴 문인 경우 플레이어가 열쇠를 들고 있는 상태인지) 재검사
	FText Failuremessage;
	if (!InteractInterface->CanStartInteraction(ASC, Failuremessage))
	{
		OwnerCharacter->Client_DisplayInteractionFailedMessage(Failuremessage);
		return;
	}

	// ----------- 어빌리티 실행 가능 ----------
	// 어빌리티 부여 및 실행
	FGameplayAbilitySpecHandle InteractAbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityToInteract, 1));
	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.Target = TargetActor;

	OwnerCharacter->CacheInteractionTarget(TargetActor);

	FGameplayAbilityActorInfo ActorInfo = *GetCurrentActorInfo();
	const bool bSuccess = ASC->TriggerAbilityFromGameplayEvent(InteractAbilityHandle, &ActorInfo, Payload.EventTag, &Payload, *ASC);
	// 실패시 복구
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Interact: TriggerAbilityFromGameplayEvent failed for ability %s."), *AbilityToInteract->GetName());

		// 실행에 실패한 일회성 어빌리티 정리
		ASC->ClearAbility(InteractAbilityHandle);
		// GA_Interact의 상태는 이미 깨끗하므로, 플레이어는 즉시 다시 상호작용을 시도 가능
		// linetrace ability 재시작 필요 x
	}
}

void UGA_Interact::HandleFailedInteractionAttempt(AActor* TargetActor)
{
	if (WaitForInteractTag)
	{
		WaitForInteractTag = nullptr;
	}

	// ----------- 유효성 검사 ------------
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
	// ----------- 유효성 검사 ------------

	// 현재 시점에서 HandAction 중이면 return
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		return;
	}

	FText FailureMessage;
	InteractableInterface->CanStartInteraction(OwnerCharacter->GetAbilitySystemComponent(), FailureMessage);
	OwnerCharacter->Client_DisplayInteractionFailedMessage(FailureMessage);
}

void UGA_Interact::UpdateInteractionUI(float Progress)
{
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (OwnerCharacter)
	{
		// Update progress bar
		OwnerCharacter->Client_UpdateInteractionProgress(Progress);
	}
}

void UGA_Interact::OnHoldInputCompleted()
{
	InteractWithTarget(CachedTargetActor.Get());
}

void UGA_Interact::OnHoldInputCancelled()
{
	WaitForHoldInputTask = nullptr;
}

/*
* 문을 계속 바라보는 상태를 유지하며 열쇠를 든 상태에서 Inventory Slot을 바꾸었을 때 
* 아무것도 들지 않아도 이전의 열쇠를 든 상태가 남아 Interaction이 가능한 판정이 되는것을 방지 
* 마찬가지로 아무것도 들지 않은 상태에서 열쇠 Inventory Slot으로 바꾸었을 때
* 아무것도 들지 않은 상태가 남아 Interaction이 불가능한 판정이 되는것을 방지
*/
void UGA_Interact::OnInventorySlotChanged(int32 NewSlotIndex)
{
	// Inventory slot이 바뀌어 기존 Task 초기화
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
