// Fill out your copyright notice in the Description page of Project Settings.


#include "PGBTTask_FindRandomLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "PGEnemyAIController.h"


UPGBTTask_FindRandomLocation::UPGBTTask_FindRandomLocation(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = "Find Random Location In NavMesh";
}

EBTNodeResult::Type UPGBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//ai ��Ʈ�ѷ� ��������
	if (APGEnemyAIController* const cont = Cast<APGEnemyAIController>(OwnerComp.GetAIOwner()))
	{
		//�ش� ai ��Ʈ�ѷ��� pawn ��������
		if (auto* const enemy = cont->GetPawn())
		{
			auto const Origin = enemy->GetActorLocation();

			if (auto* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
			{
				FNavLocation Loc;
				//�ݰ泻 ������ ��ġ ã��.
				if (NavSys->GetRandomPointInNavigableRadius(Origin, SearchRadius, Loc))
				{
					//ã�� ��ġ Blackboard Ű�� �����ϱ�. (�����Ϳ��� ���)
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Loc.Location);					
				}

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
