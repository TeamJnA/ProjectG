// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Interact.generated.h"

/**
 * 
 */

class UAT_PGWaitGameplayTagAdded;
class UAT_WaitForInteractionTarget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetUpdate, AActor*, InteractableActor);

UCLASS()
class PROJECTG_API UGA_Interact : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	UFUNCTION()
	void WaitInteractionInput(AActor* TargetActor);
	
	UFUNCTION()
	void InteractWithTarget(AActor* TargetActor);

	TSubclassOf<UGameplayAbility> AbilityToInteract;

	TObjectPtr<UAT_PGWaitGameplayTagAdded> WaitForInteractTag;

	TObjectPtr<UAT_WaitForInteractionTarget> WaitForInteractionTarget;

public:
	UPROPERTY()
	FOnInteractTargetUpdate OnInteractTargetUpdate;
};
