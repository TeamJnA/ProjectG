// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"

#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"

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
	void SetSightEnable(bool bEnable);

	FORCEINLINE float GetChaseStartDistance() const { return ChaseStartDistance; }
	FORCEINLINE float GetSanityChaseThreshold() const { return SanityChaseThreshold; }
	FORCEINLINE float GetChaseStartLimitDistance() const { return ChaseStartLimitDistance; }
	FORCEINLINE FName GetBlackboardKey_AIState() const { return BlackboardKey_AIState; }
	FORCEINLINE FName GetBlackboardKey_TargetPawn() const { return BlackboardKey_TargetPawn; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupPerceptionSystem() override;

	UFUNCTION()
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) override;

	void OnTargetSanityChanged(const FOnAttributeChangeData& Data);

	UPROPERTY()
	TObjectPtr<APGGhostCharacter> OwnerGhostCharacter;

	FDelegateHandle SanityChangedDelegateHandle;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TargetPlayerASC;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float ChaseStartDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float ChaseStartLimitDistance = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "GhostAI")
	float SanityChaseThreshold = 50.0f;

	const FName BlackboardKey_AIState = FName(TEXT("AIState"));
	const FName BlackboardKey_TargetPawn = FName(TEXT("TargetPlayerPawn"));
};
