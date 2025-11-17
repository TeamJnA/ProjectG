// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "PGBTTask_RunGameplayAbility.generated.h"

class UGameplayAbility;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTTask_RunGameplayAbility : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UPGBTTask_RunGameplayAbility();

	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityToRun;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override;
};
