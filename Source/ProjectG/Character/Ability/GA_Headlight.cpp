// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/GA_Headlight.h"
#include "AbilitySystemComponent.h"
#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"
#include "Character/PGPlayerCharacter.h"

UGA_Headlight::UGA_Headlight()
{
	InputHeadLightTag = FGameplayTag::RequestGameplayTag(FName("Input.HeadLight"));
	
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(InputHeadLightTag);
	SetAssetTags(TagContainer);

	// Gameplay.State.HeadLight
	FGameplayTag HeadLightStateTag = FGameplayTag::RequestGameplayTag(FName("Gameplay.State.HeadLight"));

	ActivationOwnedTags.AddTag(HeadLightStateTag);
	
	BlockAbilitiesWithTag.AddTag(HeadLightStateTag);
}

void UGA_Headlight::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("%s | %s ability activated."), *GetClass()->GetName(), *GetName());

	if (HasAuthority(&CurrentActivationInfo))
	{
		if (APGPlayerCharacter* Character = Cast<APGPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			Character->MC_SetHeadlightState(true);
		}
	}
}

void UGA_Headlight::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		if (APGPlayerCharacter* Character = Cast<APGPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
		{
			Character->MC_SetHeadlightState(false);
		}
	}

	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}