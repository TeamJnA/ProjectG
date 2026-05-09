// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/AI/Tasks/PGBTTask_GhostCheckHuntCondition.h"
#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Player/PGPlayerState.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

#include "Enemy/Ghost/Ability/Chase/GA_GhostChase.h"
#include "Enemy/Ghost/Ability/Track/GA_GhostTrack.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"


UPGBTTask_GhostCheckHuntCondition::UPGBTTask_GhostCheckHuntCondition()
{
	NodeName = TEXT("Check Hunt Conditions");
}

EBTNodeResult::Type UPGBTTask_GhostCheckHuntCondition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APGGhostAIController* const GhostAIC = Cast<APGGhostAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* const BB = OwnerComp.GetBlackboardComponent();
	if (!GhostAIC || !BB)
	{
		return EBTNodeResult::Failed;
	}

	APGGhostCharacter* const GhostCharacter = Cast<APGGhostCharacter>(GhostAIC->GetPawn());
	if (!GhostCharacter)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* const GhostASC = GhostCharacter->GetAbilitySystemComponent();
	APlayerState* const TargetPS = GhostCharacter->GetTargetPlayerState();
	if (!TargetPS || !GhostASC)
	{
		return EBTNodeResult::Succeeded;
	}

	APawn* const TargetPawn = TargetPS->GetPawn();
	const APGPlayerState* const PGPS = Cast<APGPlayerState>(TargetPS);
	if (!TargetPawn || !PGPS || !PGPS->GetAttributeSet())
	{
		return EBTNodeResult::Succeeded;
	}

	if (PGPS->IsDead())
	{
		UE_LOG(LogTemp, Log, TEXT("PGBTTask_CheckHuntConditions: Target player is dead."));
		BB->ClearValue(GhostAIC->GetBlackboardKey_TargetPawn());
		return EBTNodeResult::Succeeded;
	}

	const float CurrentSanity = PGPS->GetAttributeSet()->GetSanity();
	if (CurrentSanity < GhostAIC->GetSanityChaseThreshold())
	{
		BB->SetValueAsObject(GhostAIC->GetBlackboardKey_TargetPawn(), TargetPawn);

		const FVector GhostLoc = GhostCharacter->GetActorLocation();
		const FVector TargetLoc = TargetPawn->GetActorLocation();

		// 직선거리로 빠르게 1차 체크 (충분히 가까우면 NavMesh 계산 스킵)
		const float DirectDistance = FVector::Dist(GhostLoc, TargetLoc);
		if (DirectDistance < GhostAIC->GetChaseStartLimitDistance() * 0.5f)
		{
			// 직선거리만으로도 너무 가까움 -> 무조건 가깝다고 판단
			UE_LOG(LogTemp, Log, TEXT("PGBTTask_CheckHuntConditions: Too close (Direct: %.0f). Re-Exploring."), DirectDistance);
			return EBTNodeResult::Succeeded;
		}

		// NavMesh 상 거리 계산(정밀 측정)
		const float PathDistance = GhostAIC->GetNavPathDistanceToTarget(GhostLoc, TargetLoc);
		if (PathDistance < 0.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("PGBTTask_CheckHuntConditions: No valid path. Re-Exploring."));
			return EBTNodeResult::Succeeded;
		}

		if (PathDistance <= GhostAIC->GetChaseStartLimitDistance())
		{
			UE_LOG(LogTemp, Log, TEXT("PGBTTask_CheckHuntConditions: Too close (%.0f). Re-Exploring."), PathDistance);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PGBTTask_CheckHuntConditions: Starting Track (Dist: %.0f)"), PathDistance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostTrack::StaticClass(), true);
		}
	}

	return EBTNodeResult::Succeeded;
}
