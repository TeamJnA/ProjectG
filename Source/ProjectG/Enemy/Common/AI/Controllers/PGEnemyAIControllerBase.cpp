// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"

APGEnemyAIControllerBase::APGEnemyAIControllerBase(FObjectInitializer const& ObjectInitializer)
{
}

void APGEnemyAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (APGEnemyCharacterBase* const enemy = Cast<APGEnemyCharacterBase>(InPawn))
	{
		if (UBehaviorTree* const tree = enemy->GetBehaviorTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(tree->BlackboardAsset,b);
			Blackboard = b;
					
			RunBehaviorTree(tree);
		}
	}
}

void APGEnemyAIControllerBase::SetupPerceptionSystem()
{}

void APGEnemyAIControllerBase::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{}
