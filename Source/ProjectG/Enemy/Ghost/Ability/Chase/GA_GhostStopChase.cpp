// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Ability/Chase/GA_GhostStopChase.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"

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

	APGGhostCharacter* Ghost = Cast<APGGhostCharacter>(GetAvatarActorFromActorInfo());
	if (Ghost)
	{
		if (APGGhostAIController* AIC = Cast<APGGhostAIController>(Ghost->GetController()))
		{
			AIC->StopChaseDistanceCheck();

			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->ClearValue(TEXT("TargetPlayerPawn"));
				BB->SetValueAsEnum(TEXT("AIState"), (uint8)E_PGGhostState::Exploring);
				Ghost->SetGhostState(E_PGGhostState::Exploring);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

	// EndAbility 후에 Exploration 재활성화 -> Speed 145로 복원
	if (Ghost)
	{
		if (UAbilitySystemComponent* ASC = Ghost->GetAbilitySystemComponent())
		{
			ASC->TryActivateAbilityByClass(UGA_Exploration::StaticClass(), true);
		}
	}
}
