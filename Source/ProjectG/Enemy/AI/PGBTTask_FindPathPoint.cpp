// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/PGBTTask_FindPathPoint.h"
#include "ProjectG/Character/PGEnemyCharacter.h"
#include "PGEnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UPGBTTask_FindPathPoint::UPGBTTask_FindPathPoint(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = TEXT("Find Path Point");
}

EBTNodeResult::Type UPGBTTask_FindPathPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* const cont = Cast<APGEnemyAIController>(OwnerComp.GetOwner()))
	{
		if (auto* const bc = OwnerComp.GetBlackboardComponent())
		{
			//순찰할 인덱스를 블랙보드 key에서 가져오기.
			auto const Index = bc->GetValueAsInt(GetSelectedBlackboardKey());

			if (auto* enemy = Cast<APGEnemyCharacter>(cont->GetPawn()))
			{
				//index번째 위치 (local) 가져오기.
				auto const Point = enemy->GetPatrolPath()->GetPatrolPoint(Index);

				//월드 좌표계로 변환하기
				auto const GlobalPoint = enemy->GetPatrolPath()->GetActorTransform().TransformPosition(Point);

				//변환한 순찰 좌표 벡터 키에 저장
				bc->SetValueAsVector(PatrolPathVectorKey.SelectedKeyName, GlobalPoint);

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
