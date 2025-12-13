// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Exit/GA_ExitItem_HandWheel.h"
#include "PGLogChannels.h"

UGA_ExitItem_HandWheel::UGA_ExitItem_HandWheel()
{
	ItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Exit.Wheel"));

	ActivationOwnedTags.AddTag(ItemTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_ExitItem_HandWheel::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogPGAbility, Log, TEXT("ChainKey active."));

	if (!ASC)
	{
		UE_LOG(LogPGAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
}

void UGA_ExitItem_HandWheel::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
