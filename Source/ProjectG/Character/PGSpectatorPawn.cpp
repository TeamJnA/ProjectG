// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Character/PGPlayerCharacter.h"
#include "Camera/CameraActor.h"

#include "EngineUtils.h" 
#include "Net/UnrealNetwork.h"

APGSpectatorPawn::APGSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);
	bReplicates = true;

	// Get input actions
	ConstructorHelpers::FObjectFinder<UInputAction> OrbitYawActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_OrbitYaw.IA_OrbitYaw"));
	if (OrbitYawActionObj.Succeeded())
	{
		OrbitYawAction = OrbitYawActionObj.Object;
	}
}

/*
* Tick���� ��ġ ������Ʈ
*/
void APGSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Ŭ���̾�Ʈ���� Possess�� ���, Tick���� ��ġ/ȸ���� ������Ʈ
	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		UpdateSpectatorPositionAndRotation();
	}
}

void APGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// left/right
		EnhancedInputComponent->BindAction(OrbitYawAction, ETriggerEvent::Triggered, this, &APGSpectatorPawn::OnOrbitYaw);
	}
}

void APGSpectatorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSpectatorPawn, TargetToOrbit);
	DOREPLIFETIME(APGSpectatorPawn, TargetPlayerState);
}

/*
* ������ �ӵ��� ���� ��� �߽����� �˵� ȸ��
* Tick�� ���� ��ġ ����
*/
void APGSpectatorPawn::OnOrbitYaw(const FInputActionValue& Value)
{
	// Ŭ���̾�Ʈ���� �Է� ó��
	if (!IsLocallyControlled() || !bCanOrbit)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	CurrentOrbitYawAngle += LookAxisVector.X * RotationSpeed * GetWorld()->GetDeltaSeconds();
	CurrentOrbitPitchAngle -= LookAxisVector.Y * RotationSpeed * GetWorld()->GetDeltaSeconds();
	CurrentOrbitPitchAngle = FMath::Clamp(CurrentOrbitPitchAngle, -49.0f, 69.0f);
}

/*
* ������ Ÿ�ٰ��� �Ÿ�(150.0f)�� �����ϸ� Ʈ��ŷ
*/
void APGSpectatorPawn::UpdateSpectatorPositionAndRotation()
{
	if (!IsValid(TargetToOrbit))
	{
		return;
	}

	const FVector TargetLocation = TargetToOrbit->GetActorLocation() + FVector(0.0f, 0.0f, 65.0f);
	const FRotator OrbitRotation = FRotator(CurrentOrbitPitchAngle, CurrentOrbitYawAngle, 0.0f);
	const FVector RelativeVector = OrbitRotation.Vector() * CurrentOrbitDistance;
	const FVector NewLocation = TargetLocation + RelativeVector;

	// Location -> PawnLocation ������Ʈ
	SetActorLocation(NewLocation);

	// Rotation -> ControlRotation ������Ʈ
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const FVector LookAtVector = TargetLocation - NewLocation;
		const FRotator NewRotation = LookAtVector.Rotation();
		PC->SetControlRotation(NewRotation);
	}
}

/*
* ���� pawn�� ���� ��� ����
* Ŭ���̾�Ʈ���� ���� ��� ���� ���ø�����Ʈ
*/
void APGSpectatorPawn::SetSpectateTarget(const AActor* NewTarget, const APlayerState* NewTargetPlayerState)
{
	TargetToOrbit = const_cast<AActor*>(NewTarget);
	TargetPlayerState = const_cast<APlayerState*>(NewTargetPlayerState);

	// for server update
	OnRep_TargetToOrbit();
	OnRep_TargetPlayerState();
}

/*
* ���� ��� ���� �� Tick�� Ȱ��ȭ�Ͽ� ���ÿ��� ���� ����
* ���� ��� �̵��� ���� ȸ��, ������ �Ÿ� ������Ʈ
*/
void APGSpectatorPawn::OnRep_TargetToOrbit()
{
	if (IsLocallyControlled())
	{
		if (Cast<ACameraActor>(TargetToOrbit))
		{
			bCanOrbit = false;
			SetActorTickEnabled(false);
			SetActorLocation(TargetToOrbit->GetActorLocation());
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->SetControlRotation(TargetToOrbit->GetActorRotation());
			}
		}
		else if (IsValid(TargetToOrbit))
		{
			UE_LOG(LogTemp, Log, TEXT("SpectatorPawn: OnRep_TargetToOrbit called. IsLocalPlayerController: %d, Target: %s"), IsLocallyControlled(), *GetNameSafe(TargetToOrbit));
			bCanOrbit = true;
			SetActorTickEnabled(true);
			CurrentOrbitYawAngle = TargetToOrbit->GetActorRotation().Yaw;
		}
		else
		{
			bCanOrbit = false;
			SetActorTickEnabled(false);
		}
	}
}

void APGSpectatorPawn::OnRep_TargetPlayerState()
{
	OnSpectateTargetChanged.Broadcast(TargetPlayerState);
}
