// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"

#include "GameplayTagContainer.h"

#include "PGGhostAIController.generated.h"

class APGGhostCharacter;
class APGPlayerState;
class UAISenseConfig_Sight;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGhostAIController : public APGEnemyAIControllerBase
{
	GENERATED_BODY()

public:
	explicit APGGhostAIController(const FObjectInitializer& ObjectInitializer);

	void SetupTarget(APlayerState* NewTargetPS);

	FORCEINLINE float GetChaseStartDistance() const { return ChaseStartDistance; }
	FORCEINLINE float GetSanityChaseThreshold() const { return SanityChaseThreshold; }
	FORCEINLINE float GetChaseStartLimitDistance() const { return ChaseStartLimitDistance; }
	FORCEINLINE FName GetBlackboardKey_AIState() const { return BlackboardKey_AIState; }
	FORCEINLINE FName GetBlackboardKey_TargetPawn() const { return BlackboardKey_TargetPawn; }

	float GetNavPathDistanceToTarget(const FVector& StartLocation, const FVector& TargetLocation) const;

	void StartChaseDistanceCheck();
	void StopChaseDistanceCheck();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void OnTargetSanityChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void CheckChaseDistance();

	UPROPERTY()
	TObjectPtr<APGGhostCharacter> OwnerGhostCharacter;

	FDelegateHandle SanityChangedDelegateHandle;
	FTimerHandle ChaseDistanceCheckTimerHandle;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TargetPlayerASC;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float ChaseStartDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float ChaseStartLimitDistance = 2500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float SanityChaseThreshold = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float ChaseDistanceCheckInterval = 0.5f;

	const FName BlackboardKey_AIState = FName(TEXT("AIState"));
	const FName BlackboardKey_TargetPawn = FName(TEXT("TargetPlayerPawn"));
};
