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
	// 클라이언트에서 입력 처리
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
	// CurrentOrbitYawAngle과 CurrentOrbitDistance를 사용하여 NewLocation 계산
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 90.0f);

	// SpectatorPawn의 위치는 Pawn 위치를 직접 업데이트
	SetActorLocation(NewLocation);

	// PlayerController의 ControlRotation을 업데이트
	// SpectatorPawn의 카메라(뷰)는 이 ControlRotation을 따름
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// NewLocation에서 TargetLocation을 바라보는 벡터를 구하여 회전을 계산
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
