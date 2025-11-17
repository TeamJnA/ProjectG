// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Player/PGPlayerState.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

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
	SetupPerceptionSystem();
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

void APGGhostAIController::SetSightEnable(bool bEnable)
{
	if (SightConfig)
	{
		UE_LOG(LogTemp, Log, TEXT("APGGhostAIController::SetSightEnabled: Sight sense %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
		GetPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), bEnable);
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

void APGGhostAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent")));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = ChaseStartDistance;
		SightConfig->LoseSightRadius = ChaseStartDistance + 500.0f;
		SightConfig->PeripheralVisionAngleDegrees = 360.0f;
		SightConfig->SetMaxAge(5.0f);

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGGhostAIController::OnTargetDetected);
	GetPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
}

void APGGhostAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB || !OwnerGhostCharacter)
	{
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Sight>() && Stimulus.WasSuccessfullySensed() && Actor == OwnerGhostCharacter->GetTargetPlayerState()->GetPawn())
	{
		const E_PGGhostState CurrentState = (E_PGGhostState)BB->GetValueAsEnum(BlackboardKey_AIState);

		if (CurrentState == E_PGGhostState::Tracking)
		{
			if (UAbilitySystemComponent* GhostASC = OwnerGhostCharacter->GetAbilitySystemComponent())
			{
				UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Target sighted while Tracking. Upgrading to Chase."));
				GhostASC->TryActivateAbilityByClass(UGA_GhostChase::StaticClass(), true);
			}
		}
	}
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
		if (CurrentState != E_PGGhostState::Exploring)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PGGhostAIController] Sanity recovered (%.1f). Stopping hunt."), NewSanity);
			GhostASC->TryActivateAbilityByClass(UGA_GhostStopChase::StaticClass(), true);
		}
	}
}
