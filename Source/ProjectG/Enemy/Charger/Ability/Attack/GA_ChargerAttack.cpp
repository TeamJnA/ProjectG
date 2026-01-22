// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Ability/Attack/GA_ChargerAttack.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "Enemy/Charger/AI/E_PGChargerState.h"
#include "Enemy/Charger/Ability/Attack/AT_PGChargeToLocation.h"
#include "Enemy/Charger/Ability/Attack/AT_PGTurnBody.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"


UGA_ChargerAttack::UGA_ChargerAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTag ChaseTag = FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase"));
	ActivationOwnedTags.AddTag(ChaseTag);
	ActivationBlockedTags.AddTag(ChaseTag);
}

// 돌진 준비 -> 돌진 -> 돌진 종료 -> 몸통 회전
void UGA_ChargerAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Charger)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APGChargerAIController* AIC = Cast<APGChargerAIController>(Charger->GetController());
	if (!AIC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AIC->GetBlackboardComponent()->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTracking, true);

	UAnimMontage* MontageToPlay = Charger->GetChargeMontage();
	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		SectionName_Start,
		false
	);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_ChargerAttack::OnMontageEnd);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_ChargerAttack::OnMontageEnd);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_ChargerAttack::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_ChargerAttack::OnMontageInterrupted);
	MontageTask->ReadyForActivation();

	GetWorld()->GetTimerManager().SetTimer(
		AttackTimeoutTimerHandle,
		this,
		&UGA_ChargerAttack::OnAttackTimeout,
		6.0f,
		false
	);

	// Start 섹션 길이 계산
	float StartDelayTime = 0.0f;
	int32 SectionIndex = MontageToPlay->GetSectionIndex(SectionName_Start);
	if (SectionIndex != INDEX_NONE)
	{
		StartDelayTime = MontageToPlay->GetSectionLength(SectionIndex);
	}
	else
	{
		StartDelayTime = 0.4f;
	}

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, StartDelayTime);
	DelayTask->OnFinish.AddDynamic(this, &UGA_ChargerAttack::OnChargeReady);
	DelayTask->ReadyForActivation();
}

void UGA_ChargerAttack::OnChargeReady()
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

	AIC->GetBlackboardComponent()->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTracking, false);

	ChargeDestination = CalculateChargeDestination();
	Charger->SetMovementSpeed(Charger->GetChargeSpeed());

	// 돌진 Task(목표 지점까지 이동)
	UAT_PGChargeToLocation* ChargeTask = UAT_PGChargeToLocation::ChargeToLocation(this, ChargeDestination, 1.0f, 50.0f);
	ChargeTask->OnChargeFinish.AddDynamic(this, &UGA_ChargerAttack::OnChargeFinish);
	ChargeTask->ReadyForActivation();
}

FVector UGA_ChargerAttack::CalculateChargeDestination()
{
	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(GetAvatarActorFromActorInfo());
	if (!Charger)
	{
		return FVector::ZeroVector;
	}

	APGChargerAIController* AIC = Cast<APGChargerAIController>(Charger->GetController());
	if (!AIC) 
	{
		return FVector::ZeroVector;
	}

	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB)
	{
		return FVector::ZeroVector;
	}

	FVector TargetLoc = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);
	FVector StartLoc = Charger->GetActorLocation();

	FVector Direction = (TargetLoc - StartLoc);
	Direction.Z = 0.0f;
	Direction.Normalize();
	if (Direction.IsNearlyZero())
	{
		Direction = Charger->GetActorForwardVector();
	}

	// 최대 거리까지 뻗은 목적지 (가상의 끝점)
	FVector DesiredEnd = StartLoc + (Direction * MaxChargeDistance);
	FVector FinalDestination = DesiredEnd;

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSystem)
	{
		FVector HitLocation;		
		const bool bHitWall = NavSystem->NavigationRaycast(Charger, StartLoc, DesiredEnd, HitLocation, NULL, AIC);
		if (bHitWall)
		{
			// 벽에 막힘 -> 벽 앞까지 돌진
			// FinalDestination = HitLocation - (Direction * WallStopOffset);
			FinalDestination = HitLocation;
		}
		else
		{
			// 벽에 안 막힘
			FNavLocation ProjectedEnd;
			if (NavSystem->ProjectPointToNavigation(DesiredEnd, ProjectedEnd, FVector(200.f, 200.f, 500.f)))
			{
				FinalDestination = ProjectedEnd.Location;
			}
			else
			{
				// 투영 실패 시 그냥 씀 (사실상 발생 x)
				FinalDestination = DesiredEnd;
			}
		}
	}
	else
	{
		// [비상용] NavSystem이 없으면 기존 LineTrace 사용
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Charger);

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartLoc, DesiredEnd, ObjectParams, Params);
		if (bHit)
		{
			FinalDestination = HitResult.Location - (Direction * WallStopOffset);
		}
	}

	// 디버깅
	DrawDebugLine(GetWorld(), StartLoc, FinalDestination, FColor::Blue, false, 2.0f, 0, 3.0f);
	DrawDebugSphere(GetWorld(), FinalDestination, 20.0f, 12, FColor::Blue, false, 2.0f);

	return FinalDestination;
}

void UGA_ChargerAttack::OnChargeFinish()
{
	if (!IsActive())
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(AttackTimeoutTimerHandle);
	AttackTimeoutTimerHandle.Invalidate();

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

	// 정지 && Charge End Anim
	Charger->GetCharacterMovement()->StopMovementImmediately();
	Charger->SetMovementSpeed(Charger->GetPatrolSpeed());

	if (UAnimInstance* AnimInst = Charger->GetMesh()->GetAnimInstance())
	{
		if (UAnimMontage* MontageToPlay = Charger->GetChargeMontage())
		{
			AnimInst->Montage_JumpToSection(SectionName_End, MontageToPlay);
		}
	}
}

void UGA_ChargerAttack::OnMontageEnd()
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

	// 회전 목표 (Stare 때 보던 곳)
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTracking, true);
	FVector LookAtTarget = BB->GetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation);

	// 회전 태스크 실행
	UAT_PGTurnBody* TurnTask = UAT_PGTurnBody::TurnToFace(this, LookAtTarget, 2.0f);
	TurnTask->OnTurnFinish.AddDynamic(this, &UGA_ChargerAttack::OnTurnFinish);
	TurnTask->ReadyForActivation();
}

// 회전 완료 후 Attack 종료
void UGA_ChargerAttack::OnTurnFinish()
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

	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const bool bIsSightSensed = BB->GetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible);
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor));
	E_PGChargerState NextState = E_PGChargerState::Exploring;
	if (bIsSightSensed && TargetActor)
	{
		NextState = E_PGChargerState::Staring;

		BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);
		BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, TargetActor->GetActorLocation());
	}
	else
	{
		NextState = E_PGChargerState::Exploring;

		BB->SetValueAsObject(APGChargerAIController::BlackboardKey_TargetActor, nullptr);
		BB->SetValueAsVector(APGChargerAIController::BlackboardKey_TargetLocation, FVector::ZeroVector);
		BB->SetValueAsBool(APGChargerAIController::BlackboardKey_IsTargetVisible, false);
		BB->SetValueAsFloat(APGChargerAIController::BlackboardKey_AccumulatedStareTime, 0.0f);
	}

	BB->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)NextState);
	Charger->SetCurrentState(NextState);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ChargerAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_ChargerAttack::OnAttackTimeout()
{
	OnChargeFinish();
}

void UGA_ChargerAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	APGChargerCharacter* Charger = Cast<APGChargerCharacter>(ActorInfo->AvatarActor.Get());
	if (Charger)
	{
		Charger->SetMovementSpeed(Charger->GetPatrolSpeed());
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}