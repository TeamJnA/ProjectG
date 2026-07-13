// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "PGBTTask_PlayExplorationSound.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTTask_PlayExplorationSound : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UPGBTTask_PlayExplorationSound();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Sound")
	FName SoundName = FName("ENEMY_Ghost_Exploration");

	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bSkipFirst = true;
};
