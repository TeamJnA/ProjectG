// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "PGBlindAIController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnemy, Log, All);

class APGBlindCharacter;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGBlindAIController : public APGEnemyAIControllerBase
{
	GENERATED_BODY()
	
	
public:
	explicit APGBlindAIController(FObjectInitializer const& ObjectInitializer);

	void SetHearingRange(float NewRange);
	void SetHearingEnabled(bool Enable);

	void ResetHuntLevel();

protected:
	virtual void SetupPerceptionSystem() override;

	UFUNCTION()
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) override;

	virtual void OnPossess(APawn* InPawn) override;

private:


	void CalculateNoise(float Noise, FVector SourceLocation);

	class UAISenseConfig_Hearing* HearingConfig;
	class UAISenseConfig_Touch* TouchConfig;

	UPROPERTY()
	TObjectPtr<APGBlindCharacter> OwnerPawn;
};
