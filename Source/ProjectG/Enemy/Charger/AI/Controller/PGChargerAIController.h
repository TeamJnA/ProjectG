// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"

#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"

#include "PGChargerAIController.generated.h"

class UAISenseConfig_Sight;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGChargerAIController : public APGEnemyAIControllerBase
{
	GENERATED_BODY()
	
public:
	APGChargerAIController(const FObjectInitializer& ObjectInitializer);

	AActor* FindBestTargetInSight();
	AActor* DetermineBestTarget(const TArray<AActor*>& PerceivedActors);
	bool CanChargeToLocation(FVector TargetLoc, float Tolerance = 100.0f);

	static const FName BlackboardKey_AIState;
	static const FName BlackboardKey_TargetActor;
	static const FName BlackboardKey_TargetLocation;
	static const FName BlackboardKey_AccumulatedStareTime;
	static const FName BlackboardKey_IsTargetVisible;
	static const FName BlackboardKey_ExploreTargetLocation;
	static const FName BlackboardKey_IsLookBack;
	static const FName BlackboardKey_IsTracking;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void SetupPerceptionSystem() override;

	UFUNCTION()
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};
