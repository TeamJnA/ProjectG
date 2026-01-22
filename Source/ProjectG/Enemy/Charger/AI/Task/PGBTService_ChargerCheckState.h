// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PGBTService_ChargerCheckState.generated.h"

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
};
