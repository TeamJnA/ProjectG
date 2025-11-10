// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Player/PGPlayerState.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Enemy/Ghost/Ability/Chase/GA_GhostChase.h"
#include "Enemy/Ghost/Ability/Chase/GA_GhostStopChase.h"
#include "Enemy/Ghost/Ability/Track/GA_GhostTrack.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"

APGGhostAIController::APGGhostAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BlackboardKey_AIState(FName(TEXT("AIState")))
	, BlackboardKey_TargetPawn(FName(TEXT("TargetPlayerPawn")))
{
	ChaseStartDistance = 2000.0f;
	SanityChaseThreshold = 50.0f;
	AttackStartLimitDistance = 500.0f;
}

void APGGhostAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwnerGhostCharacter = Cast<APGGhostCharacter>(InPawn);
	if (!OwnerGhostCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Ghost::OnPossess: No valid ghost"));
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_CheckConditions,
		this,
		&APGGhostAIController::CheckHuntConditions,
		0.5f,
		true
	);
}

void APGGhostAIController::OnUnPossess()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CheckConditions);
	Super::OnUnPossess();
}

void APGGhostAIController::CheckHuntConditions()
{
	// valid check
	if (!OwnerGhostCharacter)
	{
		return;
	}

	UAbilitySystemComponent* GhostASC = OwnerGhostCharacter->GetAbilitySystemComponent();
	APlayerState* TargetPS = OwnerGhostCharacter->GetTargetPlayerState();
	if (!TargetPS || !GhostASC)
	{
		return;
	}
	
	APawn* TargetPawn = TargetPS->GetPawn();
	if (!TargetPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Ghost (%s) has no TargetPlayerState or ASC. Waiting..."), *GetNameSafe(OwnerGhostCharacter));
		return;
	}

	const APGPlayerState* PGPS = Cast<APGPlayerState>(TargetPS);
	if (!PGPS || !PGPS->GetAttributeSet())
	{
		return;
	}

	if (!Blackboard)
	{
		return;
	}
	// valid check

	const float CurrentSanity = PGPS->GetAttributeSet()->GetSanity();

	const E_PGGhostState CurrentState = (E_PGGhostState)Blackboard->GetValueAsEnum(BlackboardKey_AIState);

	const float Distance = FVector::Dist(OwnerGhostCharacter->GetActorLocation(), TargetPawn->GetActorLocation());

	UE_LOG(LogTemp, Log, TEXT("[PGGhostAIController] Check (%s): Sanity=%.1f, Distance=%.0f"), *GetNameSafe(OwnerGhostCharacter), CurrentSanity, Distance);

	if (CurrentSanity >= SanityChaseThreshold)
	{
		if (CurrentState != E_PGGhostState::Exploring)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Sanity recovered. Stopping all chase actions."));
			Blackboard->ClearValue(BlackboardKey_TargetPawn);
			GhostASC->TryActivateAbilityByClass(UGA_GhostStopChase::StaticClass(), true);
		}
		return;
	}

	// Sanity < 50
	Blackboard->SetValueAsObject(BlackboardKey_TargetPawn, TargetPawn);

	if (CurrentState == E_PGGhostState::Chasing)
	{
		return;
	}

	if (CurrentState == E_PGGhostState::Waiting)
	{
		return;
	}

	if (CurrentState == E_PGGhostState::Tracking)
	{
		if (Distance <= ChaseStartDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Track -> Chase transition. (Dist: %.0f)"), Distance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostChase::StaticClass(), true);
		}
		return;
	}

	if (CurrentState == E_PGGhostState::Exploring)
	{
		if (Distance <= AttackStartLimitDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Too close to start chase (Dist: %.0f). Staying in Exploration."), Distance);
		}
		else if (Distance <= ChaseStartDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Exploration -> Chase transition. (Dist: %.0f)"), Distance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostChase::StaticClass(), true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Exploration -> Track transition. (Dist: %.0f)"), Distance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostTrack::StaticClass(), true);
		}
		return;
	}

	//const bool bShouldBeChasing = (CurrentSanity < SanityChaseThreshold) && (Distance > ChaseStartDistance);
	//const bool bShouldStopChasing = (CurrentSanity >= SanityChaseThreshold);

	//if (bShouldBeChasing)
	//{
	//	if (!bIsChasing)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] AI (%s) Hunt Conditions Met! (Sanity: %.1f, Distance: %.0f). Trying to activate GA_GhostChase..."),
	//			*GetNameSafe(OwnerGhostCharacter), CurrentSanity, Distance);

	//		if (Blackboard)
	//		{
	//			Blackboard->SetValueAsObject(TEXT("TargetPlayerPawn"), TargetPawn);
	//		}
	//		GhostASC->TryActivateAbilityByClass(UGA_GhostChase::StaticClass(), true);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] AI (%s) still Hunt Conditions (Sanity: %.1f, Distance: %.0f)."),
	//			*GetNameSafe(OwnerGhostCharacter), CurrentSanity, Distance);
	//	}
	//}
	//else
	//{
	//	if (!bIsChasing)
	//	{
	//		if (Blackboard && Blackboard->GetValueAsObject(TEXT("TargetPlayerPawn")) != nullptr)
	//		{
	//			UE_LOG(LogTemp, Log, TEXT("[PGGhostAIController] AI (%s) is not chasing. Clearing TargetPlayerPawn key to ensure Exploration."), *GetNameSafe(OwnerGhostCharacter));
	//			Blackboard->ClearValue(TEXT("TargetPlayerPawn"));
	//		}

	//		if (!GhostASC->HasMatchingGameplayTag(ExploringTag))
	//		{
	//			GhostASC->TryActivateAbilityByClass(UGA_Exploration::StaticClass(), true);
	//		}
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] AI (%s) not met Hunt Conditions but in hunting proccess (Sanity: %.1f, Distance: %.0f)."),
	//			*GetNameSafe(OwnerGhostCharacter), CurrentSanity, Distance);
	//	}
	//}
}
