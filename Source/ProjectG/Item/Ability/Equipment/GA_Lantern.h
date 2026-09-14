// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Lantern.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Lantern : public UGameplayAbility
{
	GENERATED_BODY()

	UGA_Lantern();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	FGameplayTag LanternTag;
};
