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

	static UAT_PGTurnBody* TurnToFace(UGameplayAbility* OwningAbility, FVector TargetLocation, float RotationSpeed = 5.0f);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	FTurnBodyDelegate OnTurnFinish;

protected:
	FVector FaceLocation;
	float RotationInterpSpeed;
	bool bIsFinished;
};
