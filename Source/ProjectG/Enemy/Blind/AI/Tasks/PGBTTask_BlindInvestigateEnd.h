// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "PGBTTask_BlindInvestigateEnd.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTTask_BlindInvestigateEnd : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	explicit UPGBTTask_BlindInvestigateEnd(FObjectInitializer const& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
