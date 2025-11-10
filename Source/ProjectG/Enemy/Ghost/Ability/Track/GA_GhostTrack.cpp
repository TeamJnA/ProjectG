// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Ability/Track/GA_GhostTrack.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"

UGA_GhostTrack::UGA_GhostTrack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));
	AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Investigate")));
	SetAssetTags(AssetTags);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsInvestigating")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));

	static ConstructorHelpers::FObjectFinder<UClass> SpeedEffectRef(TEXT("/Game/ProjectG/Enemy/Ghost/Ability/Effect/GE_GhostTrack.GE_GhostTrack_C"));
	if (SpeedEffectRef.Object)
	{
		TrackSpeedEffectClass = SpeedEffectRef.Object;
	}
}

void UGA_GhostTrack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&CurrentActivationInfo))
	{
		if (const ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					BB->SetValueAsEnum(TEXT("AIState"), (uint8)E_PGGhostState::Tracking);
				}
			}
		}

		if (TrackSpeedEffectClass)
		{
			if (HasAuthority(&CurrentActivationInfo))
			{
				FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(TrackSpeedEffectClass);
				ActiveSpeedEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
			}
		}
	}
}

void UGA_GhostTrack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActiveSpeedEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(ActiveSpeedEffectHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
