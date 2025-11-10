// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"

#include "GameplayTagContainer.h"

#include "PGGhostAIController.generated.h"

class APGGhostCharacter;
class APGPlayerState;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGhostAIController : public APGEnemyAIControllerBase
{
	GENERATED_BODY()

public:
	explicit APGGhostAIController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void CheckHuntConditions();

	UPROPERTY()
	TObjectPtr<APGGhostCharacter> OwnerGhostCharacter;

	FTimerHandle TimerHandle_CheckConditions;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float ChaseStartDistance;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float SanityChaseThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float AttackStartLimitDistance;

	const FName BlackboardKey_AIState = FName(TEXT("AIState"));
	const FName BlackboardKey_TargetPawn = FName(TEXT("TargetPlayerPawn"));
};
