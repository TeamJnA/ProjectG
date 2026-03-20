// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/AI/Tasks/PGBTTask_BlindSetSoundState.h"
#include "AIController.h"

UPGBTTask_BlindSetSoundState::UPGBTTask_BlindSetSoundState()
{
    NodeName = "Set Sound State";
}

EBTNodeResult::Type UPGBTTask_BlindSetSoundState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        return EBTNodeResult::Failed;
    }

    APGBlindCharacter* Blind = Cast<APGBlindCharacter>(AIC->GetPawn());
    if (!Blind)
    {
        return EBTNodeResult::Failed;
    }

    Blind->SetSoundState(TargetSoundState);
    return EBTNodeResult::Succeeded;
}
