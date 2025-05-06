// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTagBase.h"
#include "AT_PGWaitGameplayTagAdded.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTagWithTargetDelegate, class AActor*, TargetActor);
/**
 * 
 */

//Changed behavior in UAbilityTask_WaitGameplayTagAdd: now broadcasts only when the tag becomes active, not if it's already active.
//Receives an actor pointer to pass through a fixed actor reference
UCLASS()
class PROJECTG_API UAT_PGWaitGameplayTagAdded : public UAbilityTask_WaitGameplayTag
{
	GENERATED_BODY()

public:
	UAT_PGWaitGameplayTagAdded(const FObjectInitializer& ObjectInitializer);

	FWaitTagWithTargetDelegate Added;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_PGWaitGameplayTagAdded* WaitGameplayTagAddWithTarget(UGameplayAbility* OwningAbility, 
		FGameplayTag Tag, 
		AActor* TargetActor,
		AActor* InOptionalExternalTarget = nullptr, 
		bool OnlyTriggerOnce = false);

	virtual void Activate() override;

	virtual void GameplayTagCallback(const FGameplayTag Tag, int32 NewCount) override;

protected:
	TWeakObjectPtr<AActor> TargetActor;
};
