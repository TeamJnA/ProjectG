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

	/**
	* 현재 시야에 감지된 액터 중 최적 타겟 반환
	* CurrentTarget이 여전히 감지 중이면, 다른 후보가 TargetSwitchDistanceThreshold 이상 더 가까울 때만 교체
	* CurrentTarget에 nullptr을 넘기면 단순 최근접 선정
	*/
	AActor* FindBestTargetInSight(AActor* CurrentTarget = nullptr);

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
	void ApplyDifficultyToSight();

	virtual void SetupPerceptionSystem() override;

	UFUNCTION()
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) override;

	UFUNCTION()
	void OnTargetForgotten(AActor* Actor);

	UPROPERTY(EditDefaultsOnly, Category = "AI|Targeting")
	float TargetSwitchDistanceThreshold = 500.0f;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	static constexpr float BaseSightRadius = 2000.0f;
	static constexpr float BaseLoseSightRadius = 2400.0f;
};
