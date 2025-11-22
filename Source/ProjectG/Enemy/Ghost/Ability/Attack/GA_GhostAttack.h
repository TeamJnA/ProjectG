// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GhostAttack.generated.h"

class APGGhostCharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_GhostAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_GhostAttack();

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
	UFUNCTION()
	void OnMontageCompleted();

	void StartAirFloat(APGGhostCharacter* Ghost);
	void StopAirFloat(APGGhostCharacter* Ghost);

	float FloatingHeight = 70.0f;
};
