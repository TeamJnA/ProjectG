// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "PGEnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	explicit APGEnemyAIController(FObjectInitializer const& ObjectInitializer);


protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	class UAISenseConfig_Sight* SightConfig;
	class UAISenseConfig_Hearing* HearingConfig;

	void SetupPerceptionSystem();

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus);
};
