// Fill out your copyright notice in the Description page of Project Settings.


#include "PGBTTask_MoveToDestination.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UPGBTTask_MoveToDestination::UPGBTTask_MoveToDestination(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Move To Destination");
}

EBTNodeResult::Type UPGBTTask_MoveToDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (APGEnemyAIControllerBase* const cont = Cast<APGEnemyAIControllerBase>(OwnerComp.GetAIOwner()))
	{
		auto const Destination = OwnerComp.GetBlackboardComponent()->GetValueAsVector(GetSelectedBlackboardKey());


		UAIBlueprintHelperLibrary::SimpleMoveToLocation(cont, Destination);

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}


	return EBTNodeResult::Failed;
}
