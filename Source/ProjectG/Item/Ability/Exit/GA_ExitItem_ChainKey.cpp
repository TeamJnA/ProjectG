// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Exit/GA_ExitItem_ChainKey.h"
#include "GA_ExitItem_ChainKey.h"
#include "PGLogChannels.h"

UGA_ExitItem_ChainKey::UGA_ExitItem_ChainKey()
{
	ItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Exit.ChainKey"));

	ActivationOwnedTags.AddTag(ItemTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_ExitItem_ChainKey::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogPGAbility, Log, TEXT("ChainKey active."));

	if (!ASC)
	{
		UE_LOG(LogPGAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
}

void UGA_ExitItem_ChainKey::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
