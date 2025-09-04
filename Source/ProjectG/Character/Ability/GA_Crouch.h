// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Crouch.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAbility_Crouch, Log, All);

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Crouch : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Crouch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CrouchMovementSpeedEffect;

	FActiveGameplayEffectHandle ActiveCrouchSpeedEffectHandle;
};
