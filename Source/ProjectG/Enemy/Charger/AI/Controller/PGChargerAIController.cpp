// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
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
}

void APGChargerAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent")));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 2000.0f;
		SightConfig->LoseSightRadius = 2400.0f;
		SightConfig->PeripheralVisionAngleDegrees = 80.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGChargerAIController::OnTargetDetected);
}

void APGChargerAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if (!Actor || !Actor->IsValidLowLevel())
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

	// AIC에서는 Visible 여부 판단, 최초 대상 설정만 수행
	// 세부 로직은 Service_ChargerCheckState에서 수행
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(BlackboardKey_TargetActor));
	bool bIsSensed = Stimulus.WasSuccessfullySensed();
	if (bIsSensed)
	{
		// [감지 성공]
		// 1. 현재 타겟 x -> 들어온 대상 타겟
		// 2. 현재 타겟 o && 다른 타겟 감지 -> 무시 (CurrentTarget 유지)
		//	  현재 타겟 o && 동일 타겟 다시 감지 -> visible 갱신
		if (CurrentTarget == nullptr)
		{
			if (IAttackableTarget* Attackable = Cast<IAttackableTarget>(Actor))
			{
				if (Attackable->IsValidAttackableTarget())
				{
					BB->SetValueAsObject(BlackboardKey_TargetActor, Actor);
					BB->SetValueAsVector(BlackboardKey_TargetLocation, Actor->GetActorLocation());
					BB->SetValueAsBool(BlackboardKey_IsTargetVisible, true);
				}
			}
		}
		else if (CurrentTarget == Actor)
		{
			BB->SetValueAsBool(BlackboardKey_IsTargetVisible, true);
		}
	}
	else
	{
		// [감지 실패 (시야에서 사라짐)]
		// 3. 사라진 게 현재 타겟 -> Visible off
		if (CurrentTarget == Actor)
		{
			BB->SetValueAsBool(BlackboardKey_IsTargetVisible, false);
		}
	}
}

AActor* APGChargerAIController::FindBestTargetInSight()
{
	if (UAIPerceptionComponent* PerceptionComp = GetPerceptionComponent())
	{
		TArray<AActor*> PerceivedActors;
		// 시야에 감지된 모든 액터 가져오기
		PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
		// 가장 가까운 유효 타겟 찾기
		return DetermineBestTarget(PerceivedActors);
	}
	return nullptr;
}

AActor* APGChargerAIController::DetermineBestTarget(const TArray<AActor*>& PerceivedActors)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) 
	{
		return nullptr;
	}

	AActor* ClosestTarget = nullptr;
	float MinDistanceSq = FLT_MAX;
	FVector MyLoc = MyPawn->GetActorLocation();

	for (AActor* PerceivedActor : PerceivedActors)
	{
		if (!PerceivedActor || !PerceivedActor->IsValidLowLevel())
		{
			continue;
		}

		// 죽은 플레이어 제외
		IAttackableTarget* Attackable = Cast<IAttackableTarget>(PerceivedActor);
		if (!Attackable || !Attackable->IsValidAttackableTarget())
		{
			continue;
		}

		// 거리 비교
		float DistSq = FVector::DistSquared(MyLoc, PerceivedActor->GetActorLocation());
		if (DistSq < MinDistanceSq)
		{
			MinDistanceSq = DistSq;
			ClosestTarget = PerceivedActor;
		}
	}

	return ClosestTarget;
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

	FVector StartLoc = Charger->GetActorLocation();
	FVector LineTraceStartLoc = Charger->GetActorLocation();
	LineTraceStartLoc.Z -= 60.0f;

	// 시작점과 목표점 사이에 끊긴 길(NavMesh 구멍)이 있는지 검사
	FVector HitLocation;
	const bool bHitWall = NavSystem->NavigationRaycast(Charger, StartLoc, TargetLoc, HitLocation, NULL, this);

	if (bHitWall)
	{
		// 막힌 지점이 목표 지점과 거의 비슷하다면(오차범위) 통과
		if (FVector::Dist2D(HitLocation, TargetLoc) < Tolerance)
		{
			return true;
		}
		// 중간에 끊김 (계단 허공 등)
		return false;
	}

	return true;
}