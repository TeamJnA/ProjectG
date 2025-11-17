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

	const float CurrentSanity = PGPS->GetAttributeSet()->GetSanity();

	if (CurrentSanity < GhostAIC->GetSanityChaseThreshold())
	{
		BB->SetValueAsObject(GhostAIC->GetBlackboardKey_TargetPawn(), TargetPawn);

		const float Distance = FVector::Dist(GhostCharacter->GetActorLocation(), TargetPawn->GetActorLocation());
				
		if (Distance <= GhostAIC->GetChaseStartLimitDistance())
		{
			UE_LOG(LogTemp, Log, TEXT("PGBTTask_CheckHuntConditions: Too close (%.0f). Re-Exploring."), Distance);
		}
		else if (Distance <= GhostAIC->GetChaseStartDistance())
		{
			UE_LOG(LogTemp, Warning, TEXT("PGBTTask_CheckHuntConditions: Starting Chase (Dist: %.0f)"), Distance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostChase::StaticClass(), true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PGBTTask_CheckHuntConditions: Starting Track (Dist: %.0f)"), Distance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostTrack::StaticClass(), true);
		}
	}

	return EBTNodeResult::Succeeded;
}
