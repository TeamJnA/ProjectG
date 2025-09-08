// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Character/PGPlayerCharacter.h"

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

	TargetToOrbit = nullptr;
	RotationSpeed = 90.0f;
	CurrentOrbitDistance = 150.0f;
	CurrentOrbitYawAngle = 0.0f;
}

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
		// left, right
		EnhancedInputComponent->BindAction(OrbitYawAction, ETriggerEvent::Triggered, this, &APGSpectatorPawn::OnOrbitYaw);
	}
}

void APGSpectatorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSpectatorPawn, TargetToOrbit);
}

void APGSpectatorPawn::OnOrbitYaw(const FInputActionValue& Value)
{
	// Ŭ���̾�Ʈ���� �Է� ó��
	if (!IsLocallyControlled()) 
	{
		return;
	}

	float AxisValue = Value.Get<float>();
	CurrentOrbitYawAngle += AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
	UE_LOG(LogTemp, Verbose, TEXT("SpectatorPawn: UpdateOrbitYawInput: CurrentOrbitYawAngle: %.2f"), CurrentOrbitYawAngle);
}

void APGSpectatorPawn::UpdateSpectatorPositionAndRotation()
{
	if (!IsValid(TargetToOrbit))
	{
		UE_LOG(LogTemp, Error, TEXT("SpectatorPawn: UpdateSpectatorPositionAndRotation: TargetToOrbit is null. Cannot update."));
		return;
	}

	FVector TargetLocation = TargetToOrbit->GetActorLocation();
	// CurrentOrbitYawAngle�� CurrentOrbitDistance�� ����Ͽ� NewLocation ���
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 90.0f);

	// SpectatorPawn�� ��ġ�� Pawn ��ġ�� ���� ������Ʈ
	SetActorLocation(NewLocation);

	// PlayerController�� ControlRotation�� ������Ʈ
	// SpectatorPawn�� ī�޶�(��)�� �� ControlRotation�� ����
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// NewLocation���� TargetLocation�� �ٶ󺸴� ���͸� ���Ͽ� ȸ���� ���
		FVector LookAtVector = TargetLocation - NewLocation;
		FRotator NewRotation = LookAtVector.Rotation();
		PC->SetControlRotation(NewRotation);
	}
}

void APGSpectatorPawn::SetSpectateTarget(AActor* NewTarget)
{
	// only call on server
	TargetToOrbit = NewTarget;

	OnRep_TargetToOrbit();
}

void APGSpectatorPawn::OnRep_TargetToOrbit()
{
	if (IsLocallyControlled())
	{
		if (IsValid(TargetToOrbit))
		{
			UE_LOG(LogTemp, Log, TEXT("SpectatorPawn: OnRep_TargetToOrbit called. IsLocalPlayerController: %d, Target: %s"), IsLocallyControlled(), *GetNameSafe(TargetToOrbit));
			SetActorTickEnabled(true);
			UpdateSpectatorPositionAndRotation();
		}
		else
		{
			SetActorTickEnabled(false);
		}
	}
}
