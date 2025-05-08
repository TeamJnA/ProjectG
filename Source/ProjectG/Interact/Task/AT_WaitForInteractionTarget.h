// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WaitForInteractionTarget.generated.h"

class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionTargetDelegate, class AActor*, TargetActor);
/**
 * 
 */

UCLASS()
class PROJECTG_API UAT_WaitForInteractionTarget : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FInteractionTargetDelegate InteractionTarget;

	UFUNCTION(BlueprintCallable, Category = "Ability|Task", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitForInteractionTarget* WaitForInteractionTarget(UGameplayAbility* OwningAbility, UCameraComponent* CameraComponent, bool ShowDebug
																	,float TraceRate = 0.1f, float TraceRange = 1000.0);

	virtual void Activate() override;

protected:
	void TraceToFindInteractable();

	float InteractableTraceRate = 0.1f;
	float InteractableTraceRange = 1000.0;

	TObjectPtr<UCameraComponent> CameraComponent;

	bool ShowDebug = false;

	FTimerHandle TimerHandle;

	virtual void OnDestroy(bool AbilityIsEnding) override;

	TWeakObjectPtr<AActor> PreviousTargetActor;
};
