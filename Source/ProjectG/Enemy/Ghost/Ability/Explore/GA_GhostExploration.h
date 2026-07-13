// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "GA_GhostExploration.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_GhostExploration : public UGA_Exploration
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;	
};
