// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/AI/Task/PGBTTask_ChargerTurnHead.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"

UPGBTTask_ChargerTurnHead::UPGBTTask_ChargerTurnHead()
{
	NodeName = TEXT("Charger Turn Head");
}

EBTNodeResult::Type UPGBTTask_ChargerTurnHead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APGChargerCharacter* ChargerCharacter = Cast<APGChargerCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!ChargerCharacter || !BB)
	{
		return EBTNodeResult::Failed;
	}

	const bool bCurrentLookBack = BB->GetValueAsBool(LookBackKey.SelectedKeyName);
	BB->SetValueAsBool(LookBackKey.SelectedKeyName, !bCurrentLookBack);

	ChargerCharacter->RotateHeadYaw();

	return EBTNodeResult::Succeeded;
}
