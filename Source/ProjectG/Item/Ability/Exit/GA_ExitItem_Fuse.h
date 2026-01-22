// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Ability/GA_Item_Use.h"
#include "GA_ExitItem_Fuse.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_ExitItem_Fuse : public UGA_Item_Use
{
	GENERATED_BODY()
	
public:
	UGA_ExitItem_Fuse();

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
