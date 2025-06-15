// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Item_Throw.generated.h"

/**
 * 
 */
DECLARE_LOG_CATEGORY_EXTERN(LogAbility, Log, All);

// GA_Item_Throw is local predicted ability.
// This ability is executed both server and client.
UCLASS()
class PROJECTG_API UGA_Item_Throw : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Item_Throw();

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
	UFUNCTION()
	void MouseLeft();

	UFUNCTION()
	void MouseRight();

	TObjectPtr<UAnimMontage> ThrowAnimMontage;

	FGameplayTag MouseRightTag;

	FGameplayTag MouseLeftTag;

	FGameplayTag HandActionTag;

	// Projectile item to spawn.
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class APGProjectileItemBase> ProjectileItem;

	// It is possible to do hand action while right-clicking.
	// In this case, we need a boolean variable to prevent behavior 
	// when releasing right-click after performing a different action during the right-click.
	bool bThrowReady;

	UFUNCTION()
	void ThrowItemComplete();

	UFUNCTION()
	void RightInputCanceled();

	//This function will be implement in child class.
	UFUNCTION()
	virtual void SpawnProjectileActor();

	UFUNCTION()
	void ThrowReadyCanceled();

	UFUNCTION(Server, Reliable)
	void RemoveItem();
};
