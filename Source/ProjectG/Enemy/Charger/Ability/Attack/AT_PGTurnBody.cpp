// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Ability/Attack/AT_PGTurnBody.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UAT_PGTurnBody::UAT_PGTurnBody(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bIsFinished = false;
}

UAT_PGTurnBody* UAT_PGTurnBody::TurnToFace(UGameplayAbility* OwningAbility, FVector TargetLocation, float TurnSpeedDegPerSec)
{
	UAT_PGTurnBody* MyObj = NewAbilityTask<UAT_PGTurnBody>(OwningAbility);
	MyObj->FaceLocation = TargetLocation;
	MyObj->TurnSpeed = FMath::Max(TurnSpeedDegPerSec, 1.0f);
	return MyObj;
}

void UAT_PGTurnBody::Activate()
{
	Super::Activate();

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		bIsFinished = true;
		EndTask();
		return;
	}

	StartRotation = Character->GetActorRotation();

	const FVector ToTarget = (FaceLocation - Character->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero())
	{
		bIsFinished = true;
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTurnFinish.Broadcast();
		}
		EndTask();
		return;
	}

	GoalRotation = ToTarget.Rotation();
	GoalRotation.Pitch = 0.0f;
	GoalRotation.Roll = 0.0f;

	const float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(GoalRotation.Yaw - StartRotation.Yaw));
	if (AngleDiff <= FinishAngleTolerance)
	{
		bIsFinished = true;
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTurnFinish.Broadcast();
		}
		EndTask();
		return;
	}

	// 각도에 비례하되 상하한을 둬서 편차를 제한
	TurnDuration = FMath::Clamp(AngleDiff / TurnSpeed, MinTurnDuration, MaxTurnDuration);
	ElapsedTime = 0.0f;
}

void UAT_PGTurnBody::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}

	Super::TickTask(DeltaTime);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		bIsFinished = true;
		EndTask();
		return;
	}

	ElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(ElapsedTime / TurnDuration, 0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 5.0f);

	// Lerp는 최단 경로로 보간
	Character->SetActorRotation(FMath::Lerp(StartRotation, GoalRotation, EasedAlpha));

	if (Alpha >= 1.0f)
	{
		bIsFinished = true;
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTurnFinish.Broadcast();
		}
		EndTask();
	}
}
