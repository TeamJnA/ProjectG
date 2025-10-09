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
* Tick마다 위치 업데이트
*/
void APGSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// 클라이언트에서 Possess된 경우, Tick마다 위치/회전을 업데이트
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
}

/*
* 설정된 속도로 관전 대상 중심으로 궤도 회전
* Tick에 의해 위치 조정
*/
void APGSpectatorPawn::OnOrbitYaw(const FInputActionValue& Value)
{
	// 클라이언트에서 입력 처리
	if (!IsLocallyControlled() || !bCanOrbit)
	{
		return;
	}

	float AxisValue = Value.Get<float>();
	CurrentOrbitYawAngle += AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
}

/*
* 설정된 타겟과의 거리(150.0f)를 유지하며 트래킹
*/
void APGSpectatorPawn::UpdateSpectatorPositionAndRotation()
{
	if (!IsValid(TargetToOrbit))
	{
		return;
	}

	FVector TargetLocation = TargetToOrbit->GetActorLocation();
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 90.0f);

	// Location -> Pawn 위치 직접 업데이트
	SetActorLocation(NewLocation);

	// Rotation -> ControlRotation 업데이트
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector LookAtVector = TargetLocation - NewLocation;
		FRotator NewRotation = LookAtVector.Rotation();
		PC->SetControlRotation(NewRotation);
	}
}

/*
* 관전 pawn의 관전 대상 설정
* 클라이언트에도 관전 대상 정보 레플리케이트
*/
void APGSpectatorPawn::SetSpectateTarget(AActor* NewTarget)
{
	TargetToOrbit = NewTarget;

	if (ACameraActor* CameraTarget = Cast<ACameraActor>(NewTarget))
	{
		bCanOrbit = false;
		SetActorTickEnabled(false);

		SetActorLocation(CameraTarget->GetActorLocation());
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetControlRotation(CameraTarget->GetActorRotation());
		}
	}
	else
	{
		bCanOrbit = true;
		SetActorTickEnabled(true);

		CurrentOrbitYawAngle = NewTarget->GetActorRotation().Yaw;

		UpdateSpectatorPositionAndRotation();
	}

	// for server update
	OnRep_TargetToOrbit();
}

/*
* 관전 대상 설정 시 Tick을 활성화하여 지연 없이 로컬에서 직접 추적
* 관전 대상 이동에 따른 회전, 대상과의 거리 업데이트
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
