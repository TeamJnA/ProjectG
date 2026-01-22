// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/AI/Task/PGBTTask_ChargerAttack.h"
#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UPGBTTask_ChargerAttack::UPGBTTask_ChargerAttack()
{
	NodeName = TEXT("Charger Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UPGBTTask_ChargerAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APGChargerAIController* AIC = Cast<APGChargerAIController>(OwnerComp.GetAIOwner());
	APGChargerCharacter* Charger = AIC ? Cast<APGChargerCharacter>(AIC->GetPawn()) : nullptr;

	if (!Charger || !AttackAbilityClass)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Charger);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	CachedASC = ASC;

	FGameplayAbilitySpecHandle SpecHandle;
	if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AttackAbilityClass))
	{
		SpecHandle = Spec->Handle;
		if (ASC->TryActivateAbility(SpecHandle))
		{
			// 어빌리티 종료 델리게이트 바인딩
			AbilityEndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &UPGBTTask_ChargerAttack::OnAbilityEnded);
			return EBTNodeResult::InProgress;
		}
	}

	return EBTNodeResult::Failed;
}

void UPGBTTask_ChargerAttack::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if (AbilityEndedData.AbilityThatEnded && AbilityEndedData.AbilityThatEnded->GetClass() == AttackAbilityClass)
	{
		if (CachedASC.IsValid())
		{
			CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
			AbilityEndedDelegateHandle.Reset();
		}

		UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(GetOuter());
		if (OwnerComp)
		{
			FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

void UPGBTTask_ChargerAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}