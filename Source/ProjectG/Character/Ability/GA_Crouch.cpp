// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/GA_Crouch.h"

#include "GameFramework/Character.h"

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
	}

	// Character Movement : Crouch
	if (ACharacter* AvatarActor = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarActor->Crouch();
		UE_LOG(LogAbility_Crouch, Log, TEXT("Make AvatarActor Crouch true"));
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

	if (ACharacter* AvatarActor = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarActor->UnCrouch();
		UE_LOG(LogAbility_Crouch, Log, TEXT("Make AvatarActor Crouch false"));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
