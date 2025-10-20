// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Consumable/GA_ReviveKit.h"
#include "GA_ReviveKit.h"

UGA_ReviveKit::UGA_ReviveKit()
{
	ReviveKitTag = FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ReviveKit"));

	ActivationOwnedTags.AddTag(ReviveKitTag);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_ReviveKit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogTemp, Log, TEXT("UGA_ReviveKit::ActivateAbility: ExitKey active."));

	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UGA_ReviveKit::ActivateAbility: AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
}

void UGA_ReviveKit::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
