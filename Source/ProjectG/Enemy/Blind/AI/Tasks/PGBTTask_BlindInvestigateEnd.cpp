// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/AI/Tasks/PGBTTask_BlindInvestigateEnd.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"


UPGBTTask_BlindInvestigateEnd::UPGBTTask_BlindInvestigateEnd(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = "Complete Investigation";
}


EBTNodeResult::Type UPGBTTask_BlindInvestigateEnd::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (APGBlindAIController* const cont = Cast<APGBlindAIController>(OwnerComp.GetAIOwner()))
	{
		cont->ResetHuntLevel();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}