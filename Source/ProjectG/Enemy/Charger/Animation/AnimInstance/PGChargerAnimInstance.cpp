// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Animation/AnimInstance/PGChargerAnimInstance.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Enemy/Charger/AI/E_PGChargerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UPGChargerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ChargerCharacter = Cast<APGChargerCharacter>(TryGetPawnOwner());
}

void UPGChargerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!ChargerCharacter)
	{
		ChargerCharacter = Cast<APGChargerCharacter>(TryGetPawnOwner());
	}

	if (ChargerCharacter)
	{
		GroundSpeed = ChargerCharacter->GetVelocity().Size2D();

		E_PGChargerState CurrentState = ChargerCharacter->GetCurrentState();
		bIsAdjusting = (CurrentState == E_PGChargerState::Adjusting);
		if (CurrentState == E_PGChargerState::Exploring)
		{
			bUseLookAtTarget = false;

			LookAtLocation = ChargerCharacter->GetActorLocation() + (ChargerCharacter->GetActorForwardVector() * 1000.0f);

			const float TargetYaw = ChargerCharacter->GetHeadYaw();
			CurrentHeadYaw = FMath::FInterpTo(CurrentHeadYaw, TargetYaw, DeltaSeconds, 1.0f);
		}
		else
		{
			bUseLookAtTarget = true;

			FVector TargetLookAt = ChargerCharacter->GetHeadLookAtTarget();

			if (TargetLookAt.IsZero())
			{
				TargetLookAt = ChargerCharacter->GetActorLocation() + (ChargerCharacter->GetActorForwardVector() * 1000.0f);
			}

			if (LookAtLocation.IsZero())
			{
				LookAtLocation = ChargerCharacter->GetActorLocation() + (ChargerCharacter->GetActorForwardVector() * 1000.0f);
			}

			LookAtLocation = FMath::VInterpTo(LookAtLocation, TargetLookAt, DeltaSeconds, InterpSpeed);
		}

        const FTransform HeadTransform = ChargerCharacter->GetMesh()->GetSocketTransform(TEXT("head"), RTS_World);
        const FVector HeadLoc = HeadTransform.GetLocation();
        const FVector ActualForward = HeadTransform.GetUnitAxis(EAxis::Y);
        const FVector ActualLookDir = HeadLoc + (ActualForward * 500.0f);
        //DrawDebugLine(GetWorld(), HeadLoc, ActualLookDir, FColor::Green, false, -1.f, 0, 2.0f);
		//DrawDebugSphere(GetWorld(), LookAtLocation, 30.0f, 12, FColor::Red, false, -1.0f, 0, 1.0f);
	}
}
