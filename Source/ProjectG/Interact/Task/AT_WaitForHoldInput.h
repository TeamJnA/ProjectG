// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WaitForHoldInput.generated.h"

// delegate for broadcast hold input progress
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoldInputProgress, float, Progress);
// delegate for broadcast hold input complete
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoldInputCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoldInputCancelled);

/**
 * 
 */
UCLASS()
class PROJECTG_API UAT_WaitForHoldInput : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnHoldInputProgress OnHoldInputProgressUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnHoldInputCompleted OnHoldInputCompleted;

	UPROPERTY()
	FOnHoldInputCancelled OnHoldInputCancelled;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitForHoldInput* WaitForHoldInput(UGameplayAbility* OwningAbility, float HoldTime);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool AbilityIsEnding) override;

protected:
	UPROPERTY()
	float HoldDuration;
	float TimeHeld;
	bool bInputHeld;

	FGameplayTag InteractInputTag;
	FDelegateHandle InputTagEventHandle;

	void OnInteractInputChanged(const FGameplayTag GameplayTag, int32 NewCount);
};
