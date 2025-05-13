// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Tasks/PGBTTask_IncrementPathIndex.h"

#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UPGBTTask_IncrementPathIndex::UPGBTTask_IncrementPathIndex(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ObjectInitializer}
{
	NodeName = TEXT("Increment Path Index");
}

EBTNodeResult::Type UPGBTTask_IncrementPathIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/*
	if (APGEnemyAIControllerBase* const cont = Cast<APGEnemyAIControllerBase>(OwnerComp.GetAIOwner()))
	{
		//�ش� ai ��Ʈ�ѷ��� pawn ��������
		if (auto* const enemy = Cast<APGEnemyCharacterBase>(cont->GetPawn()))
		{
			if (auto* const BC = OwnerComp.GetBlackboardComponent())
			{

				//�� ���� ����Ʈ ��
				auto const NoOfPoints = enemy->GetPatrolPath()->Num();
				auto const MinIndex = 0;
				auto const MaxIndex = NoOfPoints - 1;
				auto Index = BC->GetValueAsInt(GetSelectedBlackboardKey());

				//BiDirectional --> ����� ���� ��� boolean
				if (bBiDirectional)
				{
					if (Index >= MaxIndex && Direction == EDirectionType::Forward)
					{
						Direction = EDirectionType::Reverse;
					}
					else if (Index == MinIndex && Direction == EDirectionType::Reverse)
					{
						Direction = EDirectionType::Forward;
					}
				}

				BC->SetValueAsInt(GetSelectedBlackboardKey(),
					(Direction == EDirectionType::Forward ? ++Index : --Index) % NoOfPoints);

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
				
			}
		}
	}
	*/


	return EBTNodeResult::Failed;
}
