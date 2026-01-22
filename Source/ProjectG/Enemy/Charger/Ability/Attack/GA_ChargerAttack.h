// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ChargerAttack.generated.h"

class APGPlayerCharacter;
class UAT_PGChargeToLocation;

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_ChargerAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_ChargerAttack();

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
	FVector ChargeDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	FName SectionName_Start = FName("Start");

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	FName SectionName_Loop = FName("Loop");

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	FName SectionName_End = FName("End");

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float MaxChargeDistance = 3000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	float WallStopOffset = 100.0f;

	UFUNCTION()
	void OnChargeReady();

	UFUNCTION()
	void OnChargeFinish();

	UFUNCTION()
	void OnTurnFinish();

	UFUNCTION()
	void OnMontageEnd();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnAttackTimeout();

private:
	FVector CalculateChargeDestination();
	FTimerHandle AttackTimeoutTimerHandle;
};
