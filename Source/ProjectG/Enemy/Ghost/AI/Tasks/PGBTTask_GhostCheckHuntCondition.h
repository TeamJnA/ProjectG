// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "PGBTTask_GhostCheckHuntCondition.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTTask_GhostCheckHuntCondition : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UPGBTTask_GhostCheckHuntCondition();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
