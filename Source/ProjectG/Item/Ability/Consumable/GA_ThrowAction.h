// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ThrowAction.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_ThrowAction : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ThrowAction();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
private:
	UFUNCTION()
	void OnCompletedAnimMontage();

	TObjectPtr<UAnimMontage> ThrowAnimMontage;
};
