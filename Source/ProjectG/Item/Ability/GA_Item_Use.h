// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Item_Use.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Item_Use : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Item_Use();

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
	UFUNCTION()
	virtual void Use();

	UFUNCTION()
	void OnUseCanceled();

	FGameplayTag InteractTag;
	FGameplayTag HandActionTag;
	FGameplayTag KeyTag;



};
