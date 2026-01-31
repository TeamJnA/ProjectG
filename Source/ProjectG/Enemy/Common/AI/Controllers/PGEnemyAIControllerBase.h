// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "PGEnemyAIControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	explicit APGEnemyAIControllerBase(FObjectInitializer const& ObjectInitializer);

	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupPerceptionSystem();
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus);
};
