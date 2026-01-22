// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Ability/Kill/GA_ChargerKill.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetMathLibrary.h"


UGA_ChargerKill::UGA_ChargerKill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTag ChaseTag = FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase"));
	CancelAbilitiesWithTag.AddTag(ChaseTag);

	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack"));
	ActivationOwnedTags.AddTag(AttackTag);
	ActivationBlockedTags.AddTag(AttackTag);

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = AttackTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_ChargerKill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Charger)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const AActor* TargetActor = TriggerEventData->Target;
	if (TargetActor)
	{
		FVector StartLoc = Charger->GetActorLocation();
		FVector TargetLoc = TargetActor->GetActorLocation();

		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLoc, TargetLoc);
		LookAtRot.Pitch = 0.0f;
		LookAtRot.Roll = 0.0f;

		Charger->SetActorRotation(LookAtRot);
	}

	Charger->GetCharacterMovement()->StopMovementImmediately();
	Charger->GetCharacterMovement()->DisableMovement();

	UAnimMontage* MontageToPlay = Charger->GetKillMontage();

	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			1.0f,
			NAME_None,
			false
		);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_ChargerKill::OnMontageEnd);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_ChargerKill::OnMontageEnd);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_ChargerKill::OnMontageEnd);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_ChargerKill::OnMontageEnd()
{
	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(GetAvatarActorFromActorInfo());
	if (!Charger)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	APGChargerAIController* AIC = Cast<APGChargerAIController>(Charger->GetController());
	if (!AIC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	Charger->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (BB)
	{
		// (1) 죽은 타겟 정보 삭제
		BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
		BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, FVector::ZeroVector);
		BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);

		// (2) 전투 관련 플래그 초기화
		BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTracking, false);
		BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);

		// (3) 상태를 'Kill' -> 'Exploration'으로 변경
		BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Exploring);
		Charger->SetCurrentState(E_PGChargerState::Exploring);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ChargerKill::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
