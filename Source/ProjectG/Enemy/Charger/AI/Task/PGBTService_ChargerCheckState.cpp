// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/AI/Task/PGBTService_ChargerCheckState.h"
#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "Enemy/Charger/AI/E_PGChargerState.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Interface/AttackableTarget.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UPGBTService_ChargerCheckState::UPGBTService_ChargerCheckState()
{
	NodeName = TEXT("ChargerCheckState");
	Interval = 0.1f;
}

void UPGBTService_ChargerCheckState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APGChargerAIController* AIC = Cast<APGChargerAIController>(OwnerComp.GetAIOwner());
	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(AIC->GetPawn());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIC || !Charger || !BB)
	{
		return;
	}

	E_PGChargerState CurrentState = (E_PGChargerState)BB->GetValueAsEnum(APGChargerAIController::BlackboardKey_AIState);
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor));

	switch (CurrentState)
	{
		case E_PGChargerState::Exploring:
		{
			bool bIsVisible = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible);

			if (TargetActor && bIsVisible)
			{
				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Staring);
				Charger->SetCurrentState(E_PGChargerState::Staring);

				FVector PlayerLoc = TargetActor->GetActorLocation();
				BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, PlayerLoc);
				Charger->SetHeadLookAtTarget(PlayerLoc);
				BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);
			}
		}
		break;

		case E_PGChargerState::Staring:
		{
			const bool bIsVisible = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible);
			bool bTargetIsValid = false;
			// 타겟 유효성 검사 (살아있는지)
			if (TargetActor)
			{
				if (IAttackableTarget* Attackable = Cast<IAttackableTarget>(TargetActor))
				{
					bTargetIsValid = Attackable->IsValidAttackableTarget();
				}
			}

			// 타겟 유지/교체 판단
			if (TargetActor && bIsVisible && bTargetIsValid)
			{
				// 타겟 생존 && 트래킹 중 -> 타겟 유지
				BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, TargetActor->GetActorLocation());
				Charger->SetHeadLookAtTarget(TargetActor->GetActorLocation());
			}
			else
			{
				// 타겟 x || 트래킹 x || 타겟 사망 -> 대체 타겟 검색
				AActor* BestTarget = AIC->FindBestTargetInSight();
				if (BestTarget)
				{
					// 시야 내에 대체 타겟 존재 -> 교체
					if (BestTarget != TargetActor)
					{
						BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, BestTarget);
						BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, BestTarget->GetActorLocation());
						BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, true);

						TargetActor = BestTarget;
					}
					// 대체 타겟이 기존 타겟인 경우(시야에서 사라졌다가 다시 돌아옴) -> Location, visible 만 갱신
					else
					{
						BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, BestTarget->GetActorLocation());
						BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, true);
					}
					Charger->SetHeadLookAtTarget(BestTarget->GetActorLocation());
				}
				// 대체 타겟 x -> TargetActor reset, TargetLocation만 유지(Staring at last known location)
				else
				{
					BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
					BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
					const FVector LastKnownLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
					Charger->SetHeadLookAtTarget(LastKnownLoc);
				}
			}

			// NavMesh 상에서 돌진 가능한지 판단
			const FVector FinalTargetLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
			const bool bCanCharge = AIC->CanChargeToLocation(FinalTargetLoc);
			if (bCanCharge)
			{
				// 돌진 가능 -> Staring 유지
				float AccTime = BB->GetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime);
				AccTime += DeltaSeconds;
				BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, AccTime);
				if (AccTime >= 5.0f)
				{
					BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Attacking);
					Charger->SetCurrentState(E_PGChargerState::Attacking);
				}
			}
			else
			{
				// 돌진 불가능 ex) 계단, 복잡한 구조 등 직선 불가 상황
				// 위치조정(Adjusting) 상태 전환
				// StareTime은 초기화하지 않고 유지 -> 위치 잡히면 바로 돌진
				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Adjusting);
				Charger->SetCurrentState(E_PGChargerState::Adjusting);
			}
		}
		break;

		case E_PGChargerState::Adjusting:
		{
			// StareTime 누적 (Adjusting에서 돌진은 x)
			float AccTime = BB->GetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime);
			AccTime += DeltaSeconds;
			BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, AccTime);

			// 타겟 및 위치 갱신 (Staring 동일)
			// Adjusting 중에도 타겟을 계속 주시하고 위치를 업데이트해야 함
			const bool bIsVisible = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible);
			bool bTargetIsValid = false;
			if (TargetActor)
			{
				if (IAttackableTarget* Attackable = Cast<IAttackableTarget>(TargetActor))
				{
					bTargetIsValid = Attackable->IsValidAttackableTarget();
				}
			}

			if (TargetActor && bIsVisible && bTargetIsValid)
			{
				BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, TargetActor->GetActorLocation());
				Charger->SetHeadLookAtTarget(TargetActor->GetActorLocation());
			}
			else
			{
				AActor* BestTarget = AIC->FindBestTargetInSight();
				if (BestTarget)
				{
					if (BestTarget != TargetActor)
					{
						BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, BestTarget);
						BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, BestTarget->GetActorLocation());
						BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, true);
						TargetActor = BestTarget;
					}
					else
					{
						BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, BestTarget->GetActorLocation());
						BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, true);
					}
					Charger->SetHeadLookAtTarget(BestTarget->GetActorLocation());
				}
				else
				{
					BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
					BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
					const FVector LastKnownLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
					Charger->SetHeadLookAtTarget(LastKnownLoc);
				}
			}

			// 만약 Adjusting 중 위치를 못잡고 TargetLocation까지 너무 가까워진 경우 블랙보드 초기화/Exploring으로 전환
			const FVector FinalTargetLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
			const float DistSq = FVector::DistSquared(Charger->GetActorLocation(), FinalTargetLoc);
			const float ArrivalThreshold = 100.0f;
			if (DistSq <= (ArrivalThreshold * ArrivalThreshold))
			{
				BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
				BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, FVector::ZeroVector);
				BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
				BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);

				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Exploring);
				Charger->SetCurrentState(E_PGChargerState::Exploring);
			}

			// NavMesh 상에서 돌진 가능한지 판단
			// 돌진 가능 -> Staring 복귀
			// 이미 StaringTime이 충분히 누적되었다면, Staring으로 복귀하자마자 바로 Attackin으로 전환될 것
			const bool bCanCharge = AIC->CanChargeToLocation(FinalTargetLoc);
			if (bCanCharge)
			{
				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Staring);
				Charger->SetCurrentState(E_PGChargerState::Staring);
			}
		}
		break;

		case E_PGChargerState::Attacking:
		{
			// Attacking 상태에서도 Tracking이 켜져있으면(Charge 준비 동작 && Charge 종료 후 복귀동 작) 위치 업데이트
			bool bIsTracking = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTracking);
			if (bIsTracking && TargetActor)
			{
				const bool bIsVisible = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible);
				if (bIsVisible)
				{
					FVector CurrentLoc = TargetActor->GetActorLocation();
					BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, CurrentLoc);
				}
			}
			FVector ChargeTarget = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
			Charger->SetHeadLookAtTarget(ChargeTarget);
		}
		break;
	}
}
