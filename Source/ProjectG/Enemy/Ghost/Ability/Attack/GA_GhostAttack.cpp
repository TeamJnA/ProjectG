// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Ability/Attack/GA_GhostAttack.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Enemy/Ghost/AI/E_PGGhostState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UGA_GhostAttack::UGA_GhostAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsAttacking"));
	ActivationOwnedTags.AddTag(AttackingTag);

	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack"));
	ActivationOwnedTags.AddTag(AttackTag);
	ActivationBlockedTags.AddTag(AttackTag);

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Investigate")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Wait")));
}

void UGA_GhostAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("GA_GhostAttack::ActivateAbility: Activated"));

	APGGhostCharacter* Ghost = Cast<APGGhostCharacter>(GetAvatarActorFromActorInfo());
	if (!Ghost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartAirFloat(Ghost);

	if (APGGhostAIController* AIC = Cast<APGGhostAIController>(Ghost->GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsEnum(AIC->GetBlackboardKey_AIState(), (uint8)E_PGGhostState::Attacking);
		}
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("PlayGhostAttack"),
		Ghost->AttackMontage,
		1.0f
	);

	if (PlayMontageTask)
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_GhostAttack::OnMontageCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_GhostAttack::OnMontageCompleted);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_GhostAttack::OnMontageCompleted);
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_GhostAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (APGGhostCharacter* Ghost = Cast<APGGhostCharacter>(GetAvatarActorFromActorInfo()))
	{
		StopAirFloat(Ghost);

		if (APGGhostAIController* AIC = Cast<APGGhostAIController>(Ghost->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsEnum(AIC->GetBlackboardKey_AIState(), (uint8)E_PGGhostState::Waiting);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_GhostAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GhostAttack::StartAirFloat(APGGhostCharacter* Ghost)
{
	if (!Ghost)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Ghost->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Flying);

		const FVector CurrentLocation = Ghost->GetActorLocation();
		const FVector TargetLocation = CurrentLocation + FVector(0.0f, 0.0f, FloatingHeight);
		Ghost->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void UGA_GhostAttack::StopAirFloat(APGGhostCharacter* Ghost)
{
	if (!Ghost)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Ghost->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
}
