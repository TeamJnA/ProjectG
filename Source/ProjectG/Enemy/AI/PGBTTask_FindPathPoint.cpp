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
			//������ �ε����� ������ key���� ��������.
			auto const Index = bc->GetValueAsInt(GetSelectedBlackboardKey());

			if (auto* enemy = Cast<APGEnemyCharacter>(cont->GetPawn()))
			{
				//index��° ��ġ (local) ��������.
				auto const Point = enemy->GetPatrolPath()->GetPatrolPoint(Index);

				//���� ��ǥ��� ��ȯ�ϱ�
				auto const GlobalPoint = enemy->GetPatrolPath()->GetActorTransform().TransformPosition(Point);

				//��ȯ�� ���� ��ǥ ���� Ű�� ����
				bc->SetValueAsVector(PatrolPathVectorKey.SelectedKeyName, GlobalPoint);

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
