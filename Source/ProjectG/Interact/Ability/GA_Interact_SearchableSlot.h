// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interact/GA_Interact.h"
#include "GA_Interact_SearchableSlot.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Interact_SearchableSlot : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Interact_SearchableSlot();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
};
