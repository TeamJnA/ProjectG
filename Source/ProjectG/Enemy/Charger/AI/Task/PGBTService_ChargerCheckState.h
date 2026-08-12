// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PGBTService_ChargerCheckState.generated.h"


struct FPGChargerCheckStateMemory
{
	FVector LastLocation = FVector::ZeroVector;
	float StuckTime = 0.0f;
	bool bHasLastLocation = false;
};

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTService_ChargerCheckState : public UBTService
{
	GENERATED_BODY()

public:
	UPGBTService_ChargerCheckState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

	UPROPERTY(EditAnywhere, Category = "Adjust")
	float AdjustProgressThreshold = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Adjust")
	float AdjustStuckTimeLimit = 1.5f;
};
