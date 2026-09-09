// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/AI/Task/PGBTService_ChargerCheckState.h"
#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "Enemy/Charger/AI/E_PGChargerState.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Game/PGGameState.h"
#include "Interface/AttackableTarget.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


UPGBTService_ChargerCheckState::UPGBTService_ChargerCheckState()
{
	NodeName = TEXT("ChargerCheckState");
	Interval = 0.1f;
}

uint16 UPGBTService_ChargerCheckState::GetInstanceMemorySize() const
{
	return sizeof(FPGChargerCheckStateMemory);
}

AActor* UPGBTService_ChargerCheckState::UpdateTargeting(APGChargerAIController* AIC, UBlackboardComponent* BB, APGChargerCharacter* Charger) const
{
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor));

	// 항상 시야 재탐색
	AActor* BestTarget = AIC->FindBestTargetInSight(CurrentTarget);
	if (BestTarget)
	{
		if (BestTarget != CurrentTarget)
		{
			BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, BestTarget);
		}

		const FVector TargetLoc = BestTarget->GetActorLocation();
		BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, TargetLoc);
		BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, true);
		Charger->SetHeadLookAtTarget(TargetLoc);

		return BestTarget;
	}

	// 시야 내 타겟 x -> 타겟 해제, 마지막 목격 위치를 계속 응시
	BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
	BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
	Charger->SetHeadLookAtTarget(BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation));

	return nullptr;
}

void UPGBTService_ChargerCheckState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APGChargerAIController* AIC = Cast<APGChargerAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIC || !BB)
	{
		return;
	}

	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(AIC->GetPawn());
	if (!Charger)
	{
		return;
	}

	FPGChargerCheckStateMemory* Mem = CastInstanceNodeMemory<FPGChargerCheckStateMemory>(NodeMemory);

	E_PGChargerState CurrentState = (E_PGChargerState)BB->GetValueAsEnum(APGChargerAIController::BlackboardKey_AIState);
	if (CurrentState != E_PGChargerState::Adjusting)
	{
		Mem->StuckTime = 0.0f;
		Mem->bHasLastLocation = false;
	}

	switch (CurrentState)
	{
		case E_PGChargerState::Exploring:
		{
			// 현재 감지 목록으로 판정
			// Exploring 중 타겟 발견
			if (AActor* FoundTarget = AIC->FindBestTargetInSight())
			{
				const FVector TargetLoc = FoundTarget->GetActorLocation();

				BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, FoundTarget);
				BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, TargetLoc);
				BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, true);
				BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);
				Charger->SetHeadLookAtTarget(TargetLoc);

				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Staring);
				Charger->SetCurrentState(E_PGChargerState::Staring);
			}
			else if (BB->GetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor))
			{
				// 이전 상태에서 남은 스테일 타겟 정리
				BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
				BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
			}
		}
		break;

		case E_PGChargerState::Staring:
		{
			UpdateTargeting(AIC, BB, Charger);

			// NavMesh 상에서 직선 돌진이 가능한지 판정
			const FVector FinalTargetLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
			if (AIC->CanChargeToLocation(FinalTargetLoc))
			{
				float AccTime = BB->GetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime);
				AccTime += DeltaSeconds;
				BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, AccTime);

				float StareThreshold = BaseStareThreshold;
				if (APGGameState* GS = AIC->GetWorld()->GetGameState<APGGameState>())
				{
					StareThreshold *= GS->GetDifficulty().ChargerStareTimeMultiplier;
				}

				if (AccTime >= StareThreshold)
				{
					BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Attacking);
					Charger->SetCurrentState(E_PGChargerState::Attacking);
				}
			}
			else
			{
				// 돌진 불가(벽/모서리) -> 위치 조정
				// StareTime은 초기화 x (조정 완료 시 즉시 돌진)
				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Adjusting);
				Charger->SetCurrentState(E_PGChargerState::Adjusting);
			}
		}
		break;

		case E_PGChargerState::Adjusting:
		{
			float AccTime = BB->GetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime);
			AccTime += DeltaSeconds;
			BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, AccTime);

			UpdateTargeting(AIC, BB, Charger);

			const FVector FinalTargetLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);

			// 이동 진행도 체크
			const FVector CurrentLoc = Charger->GetActorLocation();
			if (Mem->bHasLastLocation && FVector::Dist2D(CurrentLoc, Mem->LastLocation) < AdjustProgressThreshold * DeltaSeconds)
			{
				Mem->StuckTime += DeltaSeconds;
			}
			else
			{
				Mem->StuckTime = 0.0f;
			}
			Mem->LastLocation = CurrentLoc;
			Mem->bHasLastLocation = true;

			const float DistSq = FVector::DistSquared(CurrentLoc, FinalTargetLoc);
			const bool bArrived = DistSq <= (ArrivalThreshold * ArrivalThreshold);
			const bool bStuck = Mem->StuckTime >= AdjustStuckTimeLimit;

			// 도착했거나 끼였는데도 각이 안 나옴 -> Adjust 포기, Explore 복귀
			if (bArrived || bStuck)
			{
				Mem->StuckTime = 0.0f;
				Mem->bHasLastLocation = false;

				BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
				BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
				BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);

				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Exploring);
				Charger->SetCurrentState(E_PGChargerState::Exploring);
				break;
			}

			// 돌진 각이 나오면 Staring 복귀 (StareTime이 차 있으면 즉시 Attack)
			if (AIC->CanChargeToLocation(FinalTargetLoc))
			{
				BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Staring);
				Charger->SetCurrentState(E_PGChargerState::Staring);
			}
		}
		break;

		case E_PGChargerState::Attacking:
		{
			// Attacking은 Staring에서 정해진 타겟/위치를 그대로 사용
			// 돌진 준비 구간(IsTracking)에서만 위치 갱신 (타겟 재선정 x)
			if (BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTracking))
			{
				AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor));
				const bool bIsVisible = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible);

				if (IsValid(TargetActor) && bIsVisible)
				{
					BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, TargetActor->GetActorLocation());
				}
			}

			Charger->SetHeadLookAtTarget(BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation));
		}
		break;

		default:
			break;
	}
}
