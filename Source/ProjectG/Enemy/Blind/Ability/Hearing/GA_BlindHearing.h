// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BlindHearing.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_BlindHearing : public UGameplayAbility
{
	GENERATED_BODY()
	
	
protected:
	/** Actually activate ability, do not call this directly */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//써야할거
	//void SetAssetTags(const FGameplayTagContainer& InAbilityTags);
};
