// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GhostChase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_GhostChase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_GhostChase();

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
	void PlayChaseSoundAndScheduleNext();

	UFUNCTION()
	void OnChaseTimerFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Ghost")
	float MinChaseDuration = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ghost")
	float MaxChaseDuration = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float ChaseSoundMinInterval = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float ChaseSoundMaxInterval = 0.6f;

	FGameplayTag ChasingTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ghost")
	TSubclassOf<UGameplayEffect> ChaseSpeedEffectClass;

	FActiveGameplayEffectHandle ActiveSpeedEffectHandle;
	FTimerHandle ChaseSoundTimerHandle;
};
