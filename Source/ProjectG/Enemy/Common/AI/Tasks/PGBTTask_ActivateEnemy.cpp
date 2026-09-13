// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Tasks/PGBTTask_ActivateEnemy.h"
#include "AIController.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"


UPGBTTask_ActivateEnemy::UPGBTTask_ActivateEnemy()
{
	NodeName = TEXT("Activate Enemy");
}

EBTNodeResult::Type UPGBTTask_ActivateEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	APGEnemyCharacterBase* Enemy = AIC ? Cast<APGEnemyCharacterBase>(AIC->GetPawn()) : nullptr;

	if (Enemy)
	{
		Enemy->ActivateEnemy();
	}
	else
	{
		UE_LOG(LogEnemyCharacter, Warning, TEXT("[ActivateEnemy] invalid pawn."));
	}

	// Failed를 돌려주면 비활성 상태로 메인 트리가 도는 어중간한 상황이 되므로 항상 성공
	return EBTNodeResult::Succeeded;
}
