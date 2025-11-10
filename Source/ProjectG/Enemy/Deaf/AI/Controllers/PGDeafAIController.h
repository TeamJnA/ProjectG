// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "PGDeafAIController.generated.h"

class APGDeafCharacter;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGDeafAIController : public APGEnemyAIControllerBase
{
	GENERATED_BODY()
	
	
public:
	explicit APGDeafAIController(FObjectInitializer const& ObjectInitializer);

	void SetSightRadius(float NewRadius);
	void SetSightAngle(float NewAngle);
	void SetSightEnabled(bool Enable);

	void ResetHuntLevel();
	

protected:
	virtual void SetupPerceptionSystem() override;

	UFUNCTION()
	virtual void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	class UAISenseConfig_Sight* sightConfig;

	class UAISenseConfig_Touch* touchConfig;

	UPROPERTY()
	TObjectPtr<APGDeafCharacter> ownerPawn;

	void AssignTargetBySight(FVector targetLocation);
};
