// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Ability/Explore/GA_GhostExploration.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"


void UGA_GhostExploration::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APGGhostCharacter* Ghost = Cast<APGGhostCharacter>(GetAvatarActorFromActorInfo());
	if (!Ghost)
	{
		return;
	}

	if (AAIController* AIC = Cast<AAIController>(Ghost->GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsEnum(TEXT("AIState"), (uint8)E_PGGhostState::Exploring);
			Ghost->SetGhostState(E_PGGhostState::Exploring);
		}
	}
}
