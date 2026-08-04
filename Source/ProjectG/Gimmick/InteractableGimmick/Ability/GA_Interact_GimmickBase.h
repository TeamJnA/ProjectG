// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Interact_GimmickBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Interact_GimmickBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Interact_GimmickBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** 상호작용 시 애님 재생 여부, child에서 자체적으로 재생하면 false */
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	bool bPlayMontage = true;
};
