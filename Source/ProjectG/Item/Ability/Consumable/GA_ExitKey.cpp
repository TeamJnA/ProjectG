// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Consumable/GA_ExitKey.h"
#include "AbilitySystemComponent.h"

UGA_ExitKey::UGA_ExitKey()
{
	ExitKeyTag = FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ExitKey"));

	ActivationOwnedTags.AddTag(ExitKeyTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_ExitKey::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogTemp, Log, TEXT("UGA_ExitKey::ActivateAbility: ExitKey active."));

	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UGA_ExitKey::ActivateAbility: AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
}

void UGA_ExitKey::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
