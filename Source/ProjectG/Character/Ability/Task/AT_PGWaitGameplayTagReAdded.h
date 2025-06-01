// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTagBase.h"
#include "AT_PGWaitGameplayTagReAdded.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitTagReAddedDelegate);

/**
 * 
 */
UCLASS()
class PROJECTG_API UAT_PGWaitGameplayTagReAdded : public UAbilityTask_WaitGameplayTag
{
	GENERATED_BODY()

public:
	FWaitTagReAddedDelegate Added;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_PGWaitGameplayTagReAdded* WaitGameplayTagReAdded(UGameplayAbility* OwningAbility,
		FGameplayTag Tag,
		AActor* InOptionalExternalTarget = nullptr,
		bool OnlyTriggerOnce = false);

	virtual void Activate() override;

	virtual void GameplayTagCallback(const FGameplayTag Tag, int32 NewCount) override;
};
