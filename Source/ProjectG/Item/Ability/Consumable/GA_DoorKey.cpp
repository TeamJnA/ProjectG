// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Consumable/GA_DoorKey.h"

UGA_DoorKey::UGA_DoorKey()
{
	KeyTag = FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Key"));

	ActivationOwnedTags.AddTag(KeyTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_DoorKey::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogTemp, Log, TEXT("key active."));

	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Add key tag."));
}

void UGA_DoorKey::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
