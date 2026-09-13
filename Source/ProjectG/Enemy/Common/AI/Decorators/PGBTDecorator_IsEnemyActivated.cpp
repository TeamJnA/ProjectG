// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Decorators/PGBTDecorator_IsEnemyActivated.h"
#include "AIController.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"


UPGBTDecorator_IsEnemyActivated::UPGBTDecorator_IsEnemyActivated()
{
	NodeName = TEXT("Is Enemy Activated");

	bAllowAbortNone = true;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
	FlowAbortMode = EBTFlowAbortMode::None;
}

bool UPGBTDecorator_IsEnemyActivated::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return false;
	}

	const APGEnemyCharacterBase* Enemy = Cast<APGEnemyCharacterBase>(AIC->GetPawn());
	return Enemy && Enemy->IsEnemyActivated();
}
