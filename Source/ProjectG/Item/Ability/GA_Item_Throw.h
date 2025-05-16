// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Item_Throw.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Item_Throw : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Item_Throw();

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
	void MouseLeft();

	UFUNCTION()
	void MouseRight();

	FGameplayTag MouseRightTag;

	FGameplayTag MouseLeftTag;
};
