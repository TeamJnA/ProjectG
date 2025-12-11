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
#include "Interface/HoldInteractProgressHandler.h"

/*
* 라인 트레이스 태스크를 활성화하여 플레이어 캐릭터 카메라 정면 탐지
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

/*
* 보고 있는 대상에 대한 처리
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
	// 플레이어 애니메이션 실행 중인 경우 입력 차단/정리
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

		CachedTargetActor = TargetActor;
	}

	// 현재 타겟이 이전 타겟과 같은 경우
	// 타겟이 없거나, 이미 어떤 상호작용 Task가 실행 중이면 새로운 Tasks 생성/제거 x
	if (!TargetActor || WaitForHoldInputTask || WaitForInteractTag)
	{
		return;
	}

	/*
	* 타겟에 따라 상호작용 Task 분기/진행
	* 타겟이 상호작용 가능한 경우
	*	타겟의 상호작용 타입이 Instant인 경우(입력 즉시 상호작용) 
	*	-> WaitGameplayTagAddWithTarget 대기, 상호작용 시도 즉시 상호작용 진행
	*	타겟의 상호작용 타입이 Hold인 경우(입력 홀드를 통한 상호작용) 
	*	-> WaitForHoldInput 대기, 상호작용 시도 시 홀딩 진행률 업데이트, 홀딩 완료 시 상호작용 진행
	* 타겟이 상호작용 불가능한 경우(대상이 문이고 잠긴 상태인데 열쇠를 들고있지 않음) 
	* -> WaitGameplayTagAddWithTarget 대기, 상호작용 시도 즉시 실패 메시지 디스플레이
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
					CachedHoldTargetActor = TargetActor;

					WaitForHoldInputTask = UAT_WaitForHoldInput::WaitForHoldInput(this, Info.HoldDuration, CachedHoldTargetActor.Get());
					WaitForHoldInputTask->OnHoldInputProgressUpdated.AddDynamic(this, &UGA_Interact::UpdateInteractionUI);
					WaitForHoldInputTask->OnHoldInputCompleted.AddDynamic(this, &UGA_Interact::OnHoldInputCompleted);
					WaitForHoldInputTask->OnHoldInputEnd.AddDynamic(this, &UGA_Interact::OnHoldInputEnded);
					
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

	// 이전 Task pointer가 남아있는 경우를 대비해 nullptr로 보장
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

	// 실패시 일회성 어빌리티 정리
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Interact: TriggerAbilityFromGameplayEvent failed for ability %s."), *AbilityToInteract->GetName());

		ASC->ClearAbility(InteractAbilityHandle);
	}
}

/*
* 실패 메시지 디스플레이
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

	// 현재 시점에서 HandAction 중이면 return
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"))))
	{
		return;
	}

	FText FailureMessage;
	InteractableInterface->CanStartInteraction(OwnerCharacter->GetAbilitySystemComponent(), FailureMessage);
	InteractableInterface->InteractionFailed();
	OwnerCharacter->Client_DisplayInteractionFailedMessage(FailureMessage);
}

/*
* 홀딩 진행률 업데이트
* 홀딩 진행률 디스플레이 위젯 업데이트
*/
void UGA_Interact::UpdateInteractionUI(float Progress)
{
	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (OwnerCharacter)
	{
		// Update progress bar
		OwnerCharacter->Client_UpdateInteractionProgress(Progress);
	}

	// Do some progress hold actions
	if (IHoldInteractProgressHandler* ProgressHander = Cast<IHoldInteractProgressHandler>(CachedHoldTargetActor))
	{
		ProgressHander->UpdateHoldProgress(Progress);
	}
}

/*
* 홀드 완료 시 상호작용 실행
*/
void UGA_Interact::OnHoldInputCompleted()
{
	InteractWithTarget(CachedTargetActor.Get());

	CachedHoldTargetActor = nullptr;
}

/*
* 홀드 중단 시 초기화
*/
void UGA_Interact::OnHoldInputCancelled()
{

}

void UGA_Interact::OnHoldInputEnded()
{
	WaitForHoldInputTask = nullptr;

	APGPlayerCharacter* OwnerCharacter = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (OwnerCharacter)
	{
		// Update progress bar
		OwnerCharacter->Client_UpdateInteractionProgress(0.0f);
	}

	if (IHoldInteractProgressHandler* ProgressHander = Cast<IHoldInteractProgressHandler>(CachedHoldTargetActor))
	{
		ProgressHander->StopHoldProress();
	}
	CachedHoldTargetActor = nullptr;
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
