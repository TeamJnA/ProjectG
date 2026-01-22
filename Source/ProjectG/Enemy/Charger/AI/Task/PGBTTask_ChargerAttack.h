// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "PGBTTask_ChargerAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTTask_ChargerAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UPGBTTask_ChargerAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;

	// 어빌리티 종료 감지용 델리게이트 핸들
	FDelegateHandle AbilityEndedDelegateHandle;

	// 어빌리티 시스템 컴포넌트 캐싱
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);
};
