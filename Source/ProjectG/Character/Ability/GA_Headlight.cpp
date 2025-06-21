// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/GA_Headlight.h"
#include "AbilitySystemComponent.h"
#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"
#include "Character/PGPlayerCharacter.h"

UGA_Headlight::UGA_Headlight()
{
	MouseLeftTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseLeft"));
}

void UGA_Headlight::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("%s | %s ability activated."), *GetClass()->GetName(), *GetName());

	UAT_PGWaitGameplayTagReAdded* WaitForLeftMouseTag = UAT_PGWaitGameplayTagReAdded::WaitGameplayTagReAdded(
		this,
		MouseLeftTag);
	WaitForLeftMouseTag->Added.AddDynamic(this, &UGA_Headlight::MouseLeft);
	WaitForLeftMouseTag->ReadyForActivation();
}

void UGA_Headlight::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}

void UGA_Headlight::MouseLeft()
{
	UE_LOG(LogTemp, Log, TEXT("MouseLeft Input. %s | %s"), *GetClass()->GetName(), *GetName());

	if (HasAuthority(&CurrentActivationInfo))
	{
		APGPlayerCharacter* Character = Cast<APGPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
		if (!Character) return;

		bIsFlashlightOn = !bIsFlashlightOn;
		Character->MC_SetFlashlightState(bIsFlashlightOn);
	}
}

void UGA_Headlight::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
