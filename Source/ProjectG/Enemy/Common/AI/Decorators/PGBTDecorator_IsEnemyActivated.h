// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "PGBTDecorator_IsEnemyActivated.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTDecorator_IsEnemyActivated : public UBTDecorator
{
	GENERATED_BODY()

public:
	UPGBTDecorator_IsEnemyActivated();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
