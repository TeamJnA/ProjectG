// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_PGChargeToLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChargeToLocationDelegate);

/**
 * 
 */
UCLASS()
class PROJECTG_API UAT_PGChargeToLocation : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_PGChargeToLocation(const FObjectInitializer& ObjectInitializer);

	static UAT_PGChargeToLocation* ChargeToLocation(UGameplayAbility* OwningAbility,
	FVector TargetLocation, 
	float SpeedScale = 1.0f, 
	float AcceptanceRadius = 50.0f);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	FChargeToLocationDelegate OnChargeFinish;

protected:
	FVector Destination;
	float MoveSpeedScale;
	float StopRadius;
	bool bIsFinished;
};
