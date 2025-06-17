// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/AI/Tasks/PGBTTask_BlindChaseEnd.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

UPGBTTask_BlindChaseEnd::UPGBTTask_BlindChaseEnd(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = "Complete Chasing";
}

EBTNodeResult::Type UPGBTTask_BlindChaseEnd::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (APGBlindAIController* const cont = Cast<APGBlindAIController>(OwnerComp.GetAIOwner()))
	{
		cont->ResetHuntLevel();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
