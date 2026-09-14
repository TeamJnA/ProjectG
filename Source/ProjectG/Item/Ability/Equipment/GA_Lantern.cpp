// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Equipment/GA_Lantern.h"

UGA_Lantern::UGA_Lantern()
{
	LanternTag = FGameplayTag::RequestGameplayTag(FName("Item.Equipment.Lantern"));

	ActivationOwnedTags.AddTag(LanternTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Lantern::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
}
