// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Exit/GA_ExitItem_Fuse.h"
#include "PGLogChannels.h"

UGA_ExitItem_Fuse::UGA_ExitItem_Fuse()
{
	ItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Exit.Fuse"));

	ActivationOwnedTags.AddTag(ItemTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_ExitItem_Fuse::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogPGAbility, Log, TEXT("ExitItem Fuse active."));

	if (!ASC)
	{
		UE_LOG(LogPGAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
}

void UGA_ExitItem_Fuse::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
