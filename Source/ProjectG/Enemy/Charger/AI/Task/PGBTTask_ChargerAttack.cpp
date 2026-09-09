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

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AttackAbilityClass);
	if (!Spec || !ASC->TryActivateAbility(Spec->Handle))
	{
		return EBTNodeResult::Failed;
	}

	CachedASC = ASC;
	AbilityEndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &UPGBTTask_ChargerAttack::OnAbilityEnded);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UPGBTTask_ChargerAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 어보트 시 바인딩을 반드시 해제
	// 남아 있으면 나중에 어빌리티가 끝날 때 이미 실행 중이 아닌 노드에서
	// FinishLatentTask가 호출되어 엉뚱한 브랜치를 종료시킴
	UnbindAbilityEnded();

	if (CachedASC.IsValid() && AttackAbilityClass)
	{
		if (FGameplayAbilitySpec* Spec = CachedASC->FindAbilitySpecFromClass(AttackAbilityClass))
		{
			CachedASC->CancelAbilityHandle(Spec->Handle);
		}
	}
	CachedASC.Reset();

	return EBTNodeResult::Aborted;
}

void UPGBTTask_ChargerAttack::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if (!AbilityEndedData.AbilityThatEnded || AbilityEndedData.AbilityThatEnded->GetClass() != AttackAbilityClass)
	{
		return;
	}

	UnbindAbilityEnded();
	CachedASC.Reset();

	if (UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(GetOuter()))
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UPGBTTask_ChargerAttack::UnbindAbilityEnded()
{
	if (CachedASC.IsValid() && AbilityEndedDelegateHandle.IsValid())
	{
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
	}
	AbilityEndedDelegateHandle.Reset();
}
