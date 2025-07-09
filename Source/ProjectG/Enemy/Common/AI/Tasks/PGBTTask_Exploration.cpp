// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Tasks/PGBTTask_Exploration.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"


UPGBTTask_Exploration::UPGBTTask_Exploration(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = "Exploration";
}

EBTNodeResult::Type UPGBTTask_Exploration::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (APGEnemyAIControllerBase* const cont = Cast<APGEnemyAIControllerBase>(OwnerComp.GetAIOwner()))
	{
		if (auto* const enemy = Cast<APGEnemyCharacterBase>(cont->GetPawn()))
		{
			//interface Ȯ��
			if (enemy->Implements<UPGAIExplorationInterface>())
			{
				IPGAIExplorationInterface* IExploration = Cast<IPGAIExplorationInterface>(enemy);
				if (IExploration)
				{
					float ExplorationRadius = IExploration->GetExplorationRadius();
					float ExplorationWaitTime = IExploration->GetExplorationWaitTime();




					OwnerComp.GetBlackboardComponent()->SetValueAsFloat("WaitTime", ExplorationWaitTime);

					auto const Origin = enemy->GetActorLocation();

					
					//Navigation system ������� Ž��
					if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
					{
						FNavLocation Loc;

						if (NavSys->GetRandomPointInNavigableRadius(Origin, ExplorationRadius, Loc))
						{
							//ã�� ��ġ Blackboard Ű�� �����ϱ�. (�����Ϳ��� ���)
							OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Loc.Location);
							FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
							return EBTNodeResult::Succeeded;
						}
					}
				}
			}
		}
	}
	return EBTNodeResult::Failed;
}
