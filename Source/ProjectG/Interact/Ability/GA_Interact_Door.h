// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Interact_Door.generated.h"

#define PG_CHECK_VALID_INTERACT(Ptr) \
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
class PROJECTG_API UGA_Interact_Door : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Interact_Door();

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
	FGameplayTag HandActionLockTag;
	TObjectPtr<UAnimMontage> ThrowAnimMontage;
};
