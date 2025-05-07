// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/Services/PGBTService_ChangeSpeed.h"

#include "AIController.h"
#include "ProjectG/Enemy/Base/PGEnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

UPGBTService_ChangeSpeed::UPGBTService_ChangeSpeed()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Change Speed");
}

void UPGBTService_ChangeSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (auto const Cont = OwnerComp.GetAIOwner())
	{
		if (auto* const enemy = Cast<APGEnemyCharacterBase>(Cont->GetPawn()))
		{
			enemy->GetCharacterMovement()->MaxWalkSpeed = Speed;
		}
	}
}
