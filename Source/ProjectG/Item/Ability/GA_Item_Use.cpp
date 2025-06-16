// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_Item_Use.h"
#include "AbilitySystemComponent.h"

UGA_Item_Use::UGA_Item_Use()
{
}

void UGA_Item_Use::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
}

void UGA_Item_Use::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Item_Use::Use()
{
}

void UGA_Item_Use::OnUseComplete()
{
}

void UGA_Item_Use::OnUseCancel()
{
}