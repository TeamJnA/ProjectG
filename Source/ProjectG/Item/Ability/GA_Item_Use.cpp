// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_Item_Use.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"

UGA_Item_Use::UGA_Item_Use()
{
    //Add Event trigger
    InteractTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	HandActionTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));	
	KeyTag = FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Key"));
    
	ActivationOwnedTags.AddTag(KeyTag);
	//Block ability when using hands.
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked")));

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Item_Use::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	UE_LOG(LogTemp, Log, TEXT("key active."));

	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Add key tag."));
}

void UGA_Item_Use::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Item_Use::Use()
{
}

void UGA_Item_Use::OnUseCanceled()
{
}
