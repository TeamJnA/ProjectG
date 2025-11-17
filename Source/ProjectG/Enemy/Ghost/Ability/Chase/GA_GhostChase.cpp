// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Ability/Chase/GA_GhostChase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Enemy/Ghost/Ability/Chase/GA_GhostPostChase.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"

UGA_GhostChase::UGA_GhostChase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	MinChaseDuration = 7.0f;
	MaxChaseDuration = 9.0f;
	ChasingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing"));

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(ChasingTag);

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));

	static ConstructorHelpers::FObjectFinder<UClass> ChaseGERef(TEXT("/Game/ProjectG/Enemy/Ghost/Ability/Effect/GE_GhostChase.GE_GhostChase_C"));
	if (ChaseGERef.Object)
	{
		ChaseSpeedEffectClass = ChaseGERef.Object;
	}
}

void UGA_GhostChase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("[GA_GhostChase] AI (%s) ACTIVATED. Applying 'AI.State.IsChasing' tag for %.1f-%.1f sec."),
		*GetNameSafe(GetAvatarActorFromActorInfo()), MinChaseDuration, MaxChaseDuration);

	if (const ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsEnum(TEXT("AIState"), (uint8)E_PGGhostState::Chasing);
			}
		}
	}

	if (ChaseSpeedEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ChaseSpeedEffectClass);
		ActiveSpeedEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}

	const float Duration = FMath::RandRange(MinChaseDuration, MaxChaseDuration);

	UAbilityTask_WaitDelay* WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
	if (WaitDelayTask)
	{
		WaitDelayTask->OnFinish.AddDynamic(this, &UGA_GhostChase::OnChaseTimerFinished);
		WaitDelayTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_GhostChase::OnChaseTimerFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GhostChase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Log, TEXT("[GA_GhostChase] AI (%s) ENDED. Removing 'AI.State.IsChasing' tag."),
		*GetNameSafe(GetAvatarActorFromActorInfo()));

	if (const ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				if (!bWasCancelled)
				{
					BB->SetValueAsEnum(TEXT("AIState"), (uint8)E_PGGhostState::Waiting);
				}
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}