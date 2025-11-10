// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Deaf/Ability/Sight/GA_DeafSight.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Deaf/AI/Controllers/PGDeafAIController.h"

UGA_DeafSight::UGA_DeafSight()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.State.Blinded")));
}

void UGA_DeafSight::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (OwnerPawn)
	{
		if (APGDeafAIController* AIC = Cast<APGDeafAIController>(OwnerPawn->GetController()))
		{
			AIC->SetSightEnabled(true);
		}
	}
}

void UGA_DeafSight::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (OwnerPawn)
	{
		if (APGDeafAIController* AIC = Cast<APGDeafAIController>(OwnerPawn->GetController()))
		{
			AIC->SetSightEnabled(false);
		}
	}
}

void UGA_DeafSight::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		const bool bSuccess = ASC->TryActivateAbility(Spec.Handle);
	}
}
