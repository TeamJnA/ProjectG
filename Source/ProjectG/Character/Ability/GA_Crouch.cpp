// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/GA_Crouch.h"

#include "GameFramework/Character.h"

#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGSoundManagerComponent.h"

#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY(LogAbility_Crouch);

UGA_Crouch::UGA_Crouch()
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.Movement.Crouch")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.State.Crouching")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.Movement")));

	static ConstructorHelpers::FClassFinder<UGameplayEffect> CrouchMovementSpeedEffectRef(TEXT("/Game/ProjectG/Character/Abilities/Crouch/GE_Crouch_MovementSpeed.GE_Crouch_MovementSpeed_C"));
	if (CrouchMovementSpeedEffectRef.Class)
	{
		CrouchMovementSpeedEffect = CrouchMovementSpeedEffectRef.Class;
	}
}

void UGA_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Apply Effect( Speed )
	if (!CrouchMovementSpeedEffect)
	{
		UE_LOG(LogAbility_Crouch, Log, TEXT("Cannot find ASC"));
		return;
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		FGameplayEffectSpecHandle CrouchSpeedEffectSpecHandle = MakeOutgoingGameplayEffectSpec(CrouchMovementSpeedEffect);
		ActiveCrouchSpeedEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CrouchSpeedEffectSpecHandle);

		//TODO Crouch start sound
	}

	// Character Movement : Crouch
	if (APGPlayerCharacter* AvatarActor = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarActor->Crouch();

		if (HasAuthority(&CurrentActivationInfo))
		{
			if (UPGSoundManagerComponent* SoundComp = AvatarActor->GetSoundManagerComponent())
			{
				const FName SoundName = AvatarActor->GetStandToCrouchSoundName();
				const FVector Location = AvatarActor->GetActorLocation();

				SoundComp->TriggerSoundForAllPlayers(SoundName, Location);
			}
		}

		UE_LOG(LogAbility_Crouch, Log, TEXT("Make AvatarActor Stand to Crouch"));
	}
}

void UGA_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		if (ActiveCrouchSpeedEffectHandle.IsValid())
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			{
				ASC->RemoveActiveGameplayEffect(ActiveCrouchSpeedEffectHandle);
			}
		}

	}

	if (APGPlayerCharacter* AvatarActor = Cast<APGPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarActor->UnCrouch();

		if (HasAuthority(&CurrentActivationInfo))
		{
			if (UPGSoundManagerComponent* SoundComp = AvatarActor->GetSoundManagerComponent())
			{
				const FName SoundName = AvatarActor->GetCrouchToStandSoundName();
				const FVector Location = AvatarActor->GetActorLocation();

				SoundComp->TriggerSoundForAllPlayers(SoundName, Location);
			}
		}

		UE_LOG(LogAbility_Crouch, Log, TEXT("Make AvatarActor Crouch To Stand"));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
