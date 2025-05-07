// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/Controllers/PGEnemyAIControllerBase.h"
#include "PGDeafAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGDeafAIController : public APGEnemyAIControllerBase
{
	GENERATED_BODY()
	
	
public:
	explicit APGDeafAIController(FObjectInitializer const& ObjectInitializer);

protected:
	virtual void SetupPerceptionSystem() override;

	UFUNCTION()
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) override;

private:
	class UAISenseConfig_Sight* SightConfig;
};
