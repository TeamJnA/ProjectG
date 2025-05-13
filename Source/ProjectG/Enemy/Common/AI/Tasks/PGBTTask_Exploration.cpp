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
	//ai 컨트롤러 가져오기
	if (APGEnemyAIControllerBase* const cont = Cast<APGEnemyAIControllerBase>(OwnerComp.GetAIOwner()))
	{
		//해당 ai 컨트롤러의 pawn 가져오기
		if (auto* const enemy = Cast<APGEnemyCharacterBase>(cont->GetPawn()))
		{
			//interface 확인
			if (enemy->Implements<UPGAIExplorationInterface>())
			{
				IPGAIExplorationInterface* IExploration = Cast<IPGAIExplorationInterface>(enemy);
				if (IExploration)
				{
					float ExplorationRadius = IExploration->GetExplorationRadius();
					float ExplorationWaitTime = IExploration->GetExplorationWaitTime();
					float ExplorationMoveSpeed = IExploration->GetExplorationMoveSpeed();

					
					//이 부분 abilitySystem 으로 옮김.
					//enemy->SetMovementSpeed(ExplorationMoveSpeed);



					OwnerComp.GetBlackboardComponent()->SetValueAsFloat("WaitTime", ExplorationWaitTime);

					auto const Origin = enemy->GetActorLocation();

					if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
					{
						FNavLocation Loc;

						//반경내 랜덤한 위치 찾기.
						if (NavSys->GetRandomPointInNavigableRadius(Origin, ExplorationRadius, Loc))
						{
							//찾은 위치 Blackboard 키에 저장하기. (에디터에서 명시)
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
