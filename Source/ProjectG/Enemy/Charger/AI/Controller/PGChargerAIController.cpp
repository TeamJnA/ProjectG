// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "Game/PGGameState.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Charger/AI/E_PGChargerState.h"
#include "Interface/AttackableTarget.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"

const FName APGChargerAIController::BlackboardKey_AIState(TEXT("AIState"));
const FName APGChargerAIController::BlackboardKey_TargetActor(TEXT("TargetActor"));
const FName APGChargerAIController::BlackboardKey_TargetLocation(TEXT("TargetLocation"));
const FName APGChargerAIController::BlackboardKey_AccumulatedStareTime(TEXT("AccumulatedStareTime"));
const FName APGChargerAIController::BlackboardKey_IsTargetVisible(TEXT("IsTargetVisible"));
const FName APGChargerAIController::BlackboardKey_ExploreTargetLocation(TEXT("ExploreTargetLocation"));
const FName APGChargerAIController::BlackboardKey_IsLookBack(TEXT("IsLookBack"));
const FName APGChargerAIController::BlackboardKey_IsTracking(TEXT("IsTracking"));

APGChargerAIController::APGChargerAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	SetupPerceptionSystem();
}

void APGChargerAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Blackboard->SetValueAsEnum(BlackboardKey_AIState, (uint8)E_PGChargerState::Exploring);
	ApplyDifficultyToSight();
}

void APGChargerAIController::ApplyDifficultyToSight()
{
	if (!SightConfig)
	{
		return;
	}

	float SightMult = 1.0f;
	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		SightMult = GS->GetDifficulty().ChargerSightRangeMultiplier;
	}

	SightConfig->SightRadius = BaseSightRadius * SightMult;
	SightConfig->LoseSightRadius = BaseLoseSightRadius * SightMult;

	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->ConfigureSense(*SightConfig);
	}
}

void APGChargerAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent")));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = BaseSightRadius;
		SightConfig->LoseSightRadius = BaseLoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = 80.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGChargerAIController::OnTargetDetected);
	GetPerceptionComponent()->OnTargetPerceptionForgotten.AddDynamic(this, &APGChargerAIController::OnTargetForgotten);
}

void APGChargerAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if (!IsValid(Actor))
	{
		return;
	}

	if (Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) 
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	// AIC에서는 Visible 갱신, 최초 대상 설정만 수행
	// 타겟 선정/교체는 Service_ChargerCheckState가 수행
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(BlackboardKey_TargetActor));
	if (Stimulus.WasSuccessfullySensed())
	{
		// [감지 성공]
		// 현재 타겟 x -> 들어온 대상 타겟
		// 현재 타겟 o && 동일 타겟 다시 감지 -> visible 갱신
		if (CurrentTarget == Actor)
		{
			BB->SetValueAsBool(BlackboardKey_IsTargetVisible, true);
			return;
		}

		if (CurrentTarget == nullptr)
		{
			IAttackableTarget* Attackable = Cast<IAttackableTarget>(Actor);
			if (Attackable && Attackable->IsValidAttackableTarget())
			{
				BB->SetValueAsObject(BlackboardKey_TargetActor, Actor);
				BB->SetValueAsVector(BlackboardKey_TargetLocation, Actor->GetActorLocation());
				BB->SetValueAsBool(BlackboardKey_IsTargetVisible, true);
			}
		}
		// CurrentTarget이 있고 다른 액터가 감지 -> 서비스에서 판정
	}
	else
	{
		// 시야에서 사라짐 -> Visible off
		if (CurrentTarget == Actor)
		{
			BB->SetValueAsBool(BlackboardKey_IsTargetVisible, false);
		}
	}
}

void APGChargerAIController::OnTargetForgotten(AActor* Actor)
{
	// 폰 파괴/소스 해제 등으로 sensed=false 업데이트 없이 사라지는 경우
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	if (Cast<AActor>(BB->GetValueAsObject(BlackboardKey_TargetActor)) == Actor)
	{
		BB->SetValueAsBool(BlackboardKey_IsTargetVisible, false);
	}
}

AActor* APGChargerAIController::FindBestTargetInSight(AActor* CurrentTarget)
{
	APawn* MyPawn = GetPawn();
	UAIPerceptionComponent* PerceptionComp = GetPerceptionComponent();
	if (!MyPawn || !PerceptionComp)
	{
		return nullptr;
	}

	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	const FVector MyLoc = MyPawn->GetActorLocation();

	AActor* Closest = nullptr;
	float ClosestDistSq = FLT_MAX;
	bool bCurrentStillPerceived = false;
	float CurrentDistSq = FLT_MAX;

	for (AActor* Perceived : PerceivedActors)
	{
		if (!IsValid(Perceived))
		{
			continue;
		}

		IAttackableTarget* Attackable = Cast<IAttackableTarget>(Perceived);
		if (!Attackable || !Attackable->IsValidAttackableTarget())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(MyLoc, Perceived->GetActorLocation());

		if (Perceived == CurrentTarget)
		{
			bCurrentStillPerceived = true;
			CurrentDistSq = DistSq;
		}

		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Perceived;
		}
	}

	// 기존 타겟이 더 이상 유효하지 않으면(시야에 없거나 범위에서 벗어남) 최근접으로 즉시 교체
	if (!bCurrentStillPerceived)
	{
		return Closest;
	}

	if (Closest == CurrentTarget)
	{
		return CurrentTarget;
	}

	// 다른 타겟이 기존 타겟보다 Threshold 이상 더 가까우면 교체
	const float CurrentDist = FMath::Sqrt(CurrentDistSq);
	const float ClosestDist = FMath::Sqrt(ClosestDistSq);
	if ((CurrentDist - ClosestDist) >= TargetSwitchDistanceThreshold)
	{
		return Closest;
	}

	return CurrentTarget;
}

bool APGChargerAIController::CanChargeToLocation(FVector TargetLoc, float Tolerance)
{
	APawn* Charger = GetPawn();
	if (!Charger)
	{
		return false;
	}

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSystem) 
	{
		return false;
	}

	// 시작점과 목표점 사이에 끊긴 길(NavMesh 구멍)이 있는지 검사
	FVector StartLoc = Charger->GetActorLocation();
	FVector HitLocation;
	const bool bHitWall = NavSystem->NavigationRaycast(Charger, StartLoc, TargetLoc, HitLocation, NULL, this);
	if (bHitWall)
	{
		// 막힌 지점이 목표 지점과 거의 비슷하다면(오차범위) 통과 -> true
		// 중간에 끊김 (계단 허공 등) -> false
		return FVector::Dist2D(HitLocation, TargetLoc) < Tolerance;
	}

	return true;
}