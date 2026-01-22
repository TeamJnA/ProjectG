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

UAT_PGTurnBody* UAT_PGTurnBody::TurnToFace(UGameplayAbility* OwningAbility, FVector TargetLocation, float RotationSpeed)
{
	UAT_PGTurnBody* MyObj = NewAbilityTask<UAT_PGTurnBody>(OwningAbility);
	MyObj->FaceLocation = TargetLocation;
	MyObj->RotationInterpSpeed = RotationSpeed;
	return MyObj;
}

void UAT_PGTurnBody::Activate()
{
	Super::Activate();

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character)
	{
		FVector Direction = (FaceLocation - Character->GetActorLocation()).GetSafeNormal2D();
		float Dot = FVector::DotProduct(Character->GetActorForwardVector(), Direction);

		if (Dot >= 0.99f)
		{
			bIsFinished = true;
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnTurnFinish.Broadcast();
			}
			EndTask();
		}
	}
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
		EndTask();
		return;
	}

	FVector StartLoc = Character->GetActorLocation();
	FRotator CurrentRot = Character->GetActorRotation();
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(StartLoc, FaceLocation);

	TargetRot.Pitch = 0.0f;
	TargetRot.Roll = 0.0f;

	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);
	Character->SetActorRotation(NewRot);
	if (Character->GetActorRotation().Equals(TargetRot, 5.0f))
	{
		bIsFinished = true;
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTurnFinish.Broadcast();
		}
		EndTask();
	}
}
