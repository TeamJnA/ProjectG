// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_PGTurnBody.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTurnBodyDelegate);

/**
 * 
 */
UCLASS()
class PROJECTG_API UAT_PGTurnBody : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UAT_PGTurnBody(const FObjectInitializer& ObjectInitializer);

	// static UAT_PGTurnBody* TurnToFace(UGameplayAbility* OwningAbility, FVector TargetLocation, float RotationSpeed = 5.0f);
	static UAT_PGTurnBody* TurnToFace(UGameplayAbility* OwningAbility, FVector TargetLocation, float TurnSpeedDegPerSec = 360.0f);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	FTurnBodyDelegate OnTurnFinish;

protected:
	FVector FaceLocation;
	FRotator StartRotation;
	FRotator GoalRotation;
	float TurnDuration;
	float ElapsedTime;
	float TurnSpeed;
	bool bIsFinished;

	static constexpr float MinTurnDuration = 0.15f;
	static constexpr float MaxTurnDuration = 0.8f;
	static constexpr float FinishAngleTolerance = 1.0f;
};
