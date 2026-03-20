// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "PGBTTask_BlindSetSoundState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTTask_BlindSetSoundState : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
    UPGBTTask_BlindSetSoundState();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Sound")
    EBlindSoundState TargetSoundState = EBlindSoundState::Chasing;
	
};
