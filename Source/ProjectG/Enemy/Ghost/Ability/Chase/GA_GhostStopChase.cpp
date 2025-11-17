// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Ability/Chase/GA_GhostStopChase.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"

UGA_GhostStopChase::UGA_GhostStopChase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Investigate")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Wait")));
}

void UGA_GhostStopChase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (const ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AAIController* AIC = Cast<AAIController>(Char->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->ClearValue(TEXT("TargetPlayerPawn"));
				BB->SetValueAsEnum(TEXT("AIState"), (uint8)E_PGGhostState::Exploring);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
