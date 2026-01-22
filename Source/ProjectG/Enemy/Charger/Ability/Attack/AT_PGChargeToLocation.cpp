// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Ability/Attack/AT_PGChargeToLocation.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAT_PGChargeToLocation::UAT_PGChargeToLocation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	bTickingTask = true;
	bIsFinished = false;
}

UAT_PGChargeToLocation* UAT_PGChargeToLocation::ChargeToLocation(UGameplayAbility* OwningAbility, FVector TargetLocation, float SpeedScale, float AcceptanceRadius)
{
	UAT_PGChargeToLocation* MyObj = NewAbilityTask<UAT_PGChargeToLocation>(OwningAbility);
	MyObj->Destination = TargetLocation;
	MyObj->MoveSpeedScale = SpeedScale;
	MyObj->StopRadius = AcceptanceRadius;
	return MyObj;
}

void UAT_PGChargeToLocation::Activate()
{
	Super::Activate();

	AActor* Avatar = GetAvatarActor();
	if (Avatar)
	{
		const float DistanceSq = FVector::DistSquared2D(Avatar->GetActorLocation(), Destination);
		if (DistanceSq <= (StopRadius * StopRadius))
		{
			bIsFinished = true;
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnChargeFinish.Broadcast();
			}
			EndTask();
		}
	}
}

void UAT_PGChargeToLocation::TickTask(float DeltaTime)
{
	if (bIsFinished) 
	{
		return;
	}

	Super::TickTask(DeltaTime);

	ACharacter* MyCharacter = Cast<ACharacter>(GetAvatarActor());
	if (!MyCharacter)
	{
		EndTask();
		return;
	}

	FVector CurrentLoc = MyCharacter->GetActorLocation();
	FVector GoalLoc = Destination;

	CurrentLoc.Z = 0.0f;
	GoalLoc.Z = 0.0f;

	float DistSq = FVector::DistSquared(CurrentLoc, GoalLoc);
	if (DistSq <= (StopRadius * StopRadius))
	{
		bIsFinished = true;
		MyCharacter->ConsumeMovementInputVector();

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnChargeFinish.Broadcast();
		}
		EndTask();
		return;
	}

	FVector Direction = (GoalLoc - CurrentLoc).GetSafeNormal();
	MyCharacter->AddMovementInput(Direction, MoveSpeedScale);
}
