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

#include "NavigationSystem.h"
#include "NavigationPath.h"


APGGhostAIController::APGGhostAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BlackboardKey_AIState(FName(TEXT("AIState")))
	, BlackboardKey_TargetPawn(FName(TEXT("TargetPlayerPawn")))
{
}

void APGGhostAIController::SetupTarget(APlayerState* NewTargetPS)
{
	if (!HasAuthority() || !OwnerGhostCharacter)
	{
		return;
	}

	if (TargetPlayerASC.IsValid() && SanityChangedDelegateHandle.IsValid())
	{
		if (APGPlayerState* OldPS = Cast<APGPlayerState>(TargetPlayerASC->GetOwner()))
		{
			if (const UPGAttributeSet* OldAS = OldPS->GetAttributeSet())
			{
				TargetPlayerASC->GetGameplayAttributeValueChangeDelegate(OldAS->GetSanityAttribute()).Remove(SanityChangedDelegateHandle);
			}
		}
	}
	SanityChangedDelegateHandle.Reset();
	TargetPlayerASC.Reset();

	APGPlayerState* TargetPS = Cast<APGPlayerState>(NewTargetPS);
	if (!TargetPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ghost::SetupTargetCallbacks: No valid TargetPS."));
		return;
	}

	UAbilitySystemComponent* TargetASC = TargetPS->GetAbilitySystemComponent();
	const UPGAttributeSet* TargetAttributeSet = TargetPS->GetAttributeSet();

	if (TargetASC && TargetAttributeSet)
	{
		UE_LOG(LogTemp, Log, TEXT("Ghost::SetupTargetCallbacks: bind delegate to target sanity."), *TargetPS->GetPlayerName());
		TargetPlayerASC = TargetASC;
		SanityChangedDelegateHandle =
			TargetASC->GetGameplayAttributeValueChangeDelegate(TargetAttributeSet->GetSanityAttribute()).AddUObject(this, &APGGhostAIController::OnTargetSanityChanged);

		FOnAttributeChangeData InitData;
		InitData.NewValue = TargetAttributeSet->GetSanity();
		InitData.OldValue = InitData.NewValue;
		OnTargetSanityChanged(InitData);
	}
}

void APGGhostAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!HasAuthority())
	{
		return;
	}

	OwnerGhostCharacter = Cast<APGGhostCharacter>(InPawn);
	if (!OwnerGhostCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Ghost::OnPossess: No valid ghost"));
		return;
	}

	APlayerState* ExistingTargetPS = OwnerGhostCharacter->GetTargetPlayerState();
	if (ExistingTargetPS)
	{
		UE_LOG(LogTemp, Log, TEXT("APGGhostAIController::OnPossess: TargetPS is valid"));
		SetupTarget(ExistingTargetPS);
	}
}

void APGGhostAIController::OnUnPossess()
{
	StopChaseDistanceCheck();

	if (HasAuthority())
	{
		if (TargetPlayerASC.IsValid() && SanityChangedDelegateHandle.IsValid())
		{
			if (APGPlayerState* OldPS = Cast<APGPlayerState>(TargetPlayerASC->GetOwner()))
			{
				if (const UPGAttributeSet* OldAS = OldPS->GetAttributeSet())
				{
					TargetPlayerASC->GetGameplayAttributeValueChangeDelegate(OldAS->GetSanityAttribute()).Remove(SanityChangedDelegateHandle);
				}
			}
		}
	}

	SanityChangedDelegateHandle.Reset();
	TargetPlayerASC.Reset();

	Super::OnUnPossess();
}

void APGGhostAIController::OnTargetSanityChanged(const FOnAttributeChangeData& Data)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB || !OwnerGhostCharacter)
	{
		return;
	}

	UAbilitySystemComponent* GhostASC = OwnerGhostCharacter->GetAbilitySystemComponent();
	if (!GhostASC)
	{
		return;
	}

	const float NewSanity = Data.NewValue;
	const E_PGGhostState CurrentState = (E_PGGhostState)BB->GetValueAsEnum(BlackboardKey_AIState);

	UE_LOG(LogTemp, Log, TEXT("[PGGhostAIController] OnTargetSanityChanged: NewSanity=%.1f, CurrentState=%d"), NewSanity, (uint8)CurrentState);

	if (NewSanity >= SanityChaseThreshold)
	{
		// Attacking, Exploring 상태는 제외
		if (CurrentState != E_PGGhostState::Attacking && CurrentState != E_PGGhostState::Exploring)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Sanity recovered (%.1f). Stopping hunt."), NewSanity);
			GhostASC->TryActivateAbilityByClass(UGA_GhostStopChase::StaticClass(), true);
		}
	}
}

void APGGhostAIController::StartChaseDistanceCheck()
{
	GetWorld()->GetTimerManager().SetTimer(ChaseDistanceCheckTimerHandle, this, &APGGhostAIController::CheckChaseDistance, ChaseDistanceCheckInterval, true);
}

void APGGhostAIController::StopChaseDistanceCheck()
{
	GetWorld()->GetTimerManager().ClearTimer(ChaseDistanceCheckTimerHandle);
}

void APGGhostAIController::CheckChaseDistance()
{
	UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] CheckChaseDistance"));
	if (!OwnerGhostCharacter)
	{
		return;
	}

	APlayerState* TargetPS = OwnerGhostCharacter->GetTargetPlayerState();
	if (!TargetPS || !TargetPS->GetPawn())
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	const E_PGGhostState CurrentState = (E_PGGhostState)BB->GetValueAsEnum(BlackboardKey_AIState);
	if (CurrentState != E_PGGhostState::Tracking)
	{
		return;
	}

	const FVector GhostLoc = OwnerGhostCharacter->GetActorLocation();
	const FVector TargetLoc = TargetPS->GetPawn()->GetActorLocation();

	// 직선거리로 빠르게 1차 체크 (충분히 멀면 NavMesh 계산 없이 Tracking 유지)
	const float DirectDistance = FVector::Dist(GhostLoc, TargetLoc);
	if (DirectDistance > ChaseStartDistance * 2.0f)
	{
		return;
	}

	const float PathDistance = GetNavPathDistanceToTarget(GhostLoc, TargetLoc);
	if (PathDistance < 0.0f)
	{
		return;
	}

	if (PathDistance <= ChaseStartDistance)
	{
		if (UAbilitySystemComponent* GhostASC = OwnerGhostCharacter->GetAbilitySystemComponent())
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Target within chase distance (Path: %.0f). Upgrading to Chase."), PathDistance);
			GhostASC->TryActivateAbilityByClass(UGA_GhostChase::StaticClass(), true);
		}
	}
}

float APGGhostAIController::GetNavPathDistanceToTarget(const FVector& StartLocation, const FVector& TargetLocation) const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return -1.0f;
	}

	UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), StartLocation, TargetLocation);
	if (!NavPath || !NavPath->IsValid() || NavPath->IsPartial())
	{
		return -1.0f;
	}

	return NavPath->GetPathLength();
}
