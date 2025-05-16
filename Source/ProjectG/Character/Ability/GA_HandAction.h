// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_HandAction.generated.h"

#define PG_CHECK_VALID_HANDACTION(Ptr) \
    if (!(Ptr)) \
    { \
        UE_LOG(LogTemp, Warning, TEXT(#Ptr " is null.")); \
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); \
        return; \
    }

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_HandAction : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_HandAction();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:

	UFUNCTION()
	void OnCompletedAnimMontage();
};
