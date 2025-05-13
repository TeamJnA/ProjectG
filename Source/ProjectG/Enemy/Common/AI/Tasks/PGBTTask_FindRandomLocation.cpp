// Fill out your copyright notice in the Description page of Project Settings.


#include "PGBTTask_FindRandomLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"


UPGBTTask_FindRandomLocation::UPGBTTask_FindRandomLocation(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = "Find Random Location In NavMesh";
}

EBTNodeResult::Type UPGBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//ai 컨트롤러 가져오기
	if (APGEnemyAIControllerBase* const cont = Cast<APGEnemyAIControllerBase>(OwnerComp.GetAIOwner()))
	{
		//해당 ai 컨트롤러의 pawn 가져오기
		if (auto* const enemy = cont->GetPawn())
		{
			auto const Origin = enemy->GetActorLocation();

			if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				FNavLocation Loc;
				//반경내 랜덤한 위치 찾기.
				if (NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, Loc))
				{
					//찾은 위치 Blackboard 키에 저장하기. (에디터에서 명시)
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Loc.Location);					
				}

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
