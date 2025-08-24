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
class UAT_WaitForHoldInput;

UCLASS()
class PROJECTG_API UGA_Interact : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	UFUNCTION()
	void WaitInteractionInput(AActor* TargetActor);
	
	UFUNCTION()
	void InteractWithTarget(AActor* TargetActor);

	// ��ȣ�ۿ� �Ұ����� ��Ȳ���� ��ȣ�ۿ�� ȣ��
	UFUNCTION()
	void HandleFailedInteractionAttempt(AActor* TargetActor);

	UFUNCTION()
	void UpdateInteractionUI(float Progress);

	UFUNCTION()
	void OnHoldInputCompleted();

	UFUNCTION()
	void OnHoldInputCancelled();

	TSubclassOf<UGameplayAbility> AbilityToInteract;

	TObjectPtr<UAT_PGWaitGameplayTagAdded> WaitForInteractTag;

	TObjectPtr<UAT_WaitForInteractionTarget> WaitForInteractionTarget;

	TObjectPtr<UAT_WaitForHoldInput> WaitForHoldInputTask;

	UPROPERTY()
	TWeakObjectPtr<AActor> CachedTargetActor;

	// bind on inventory slot change
	UFUNCTION()
	void OnInventorySlotChanged(int32 NewSlotIndex);
};
