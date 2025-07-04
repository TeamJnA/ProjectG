// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

#include "Net/UnrealNetwork.h"

APGSpectatorPawn::APGSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	// 네트워크 리플리케이션 설정
	bReplicates = true;
	bAlwaysRelevant = true; // 항상 클라이언트에게 관련성이 있도록 설정 (선택 사항)

	RotationSpeed = 90.0f; // 기본 회전 속도
	TargetToOrbit = nullptr;

	// 새로 추가된 부분
	CurrentOrbitDistance = 100.0f; // 관전자와 대상 간의 초기 거리 (원하는 값으로 설정)
	CurrentOrbitYawAngle = 0.0f; // 초기 Yaw 각도
}

void APGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APGSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// 클라이언트에서 Possess된 경우, Tick마다 위치/회전을 업데이트합니다.
	// 이것이 부드러운 관전 움직임을 위한 핵심입니다.
	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		UpdateSpectatorPositionAndRotation();
		//UE_LOG(LogTemp, Verbose, TEXT("SpectatorPawn: Tick is firing and updating position."));
	}
}

void APGSpectatorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSpectatorPawn, TargetToOrbit); // TargetToOrbit을 복제하도록 설정

}

void APGSpectatorPawn::SetTargetActor(AActor* NewTarget)
{	
	// 이 함수는 주로 서버에서 TargetToOrbit 값을 설정하는 용도로 사용됩니다.
	// TargetToOrbit은 ReplicatedUsing=OnRep_TargetToOrbit이므로, 클라이언트에서는 OnRep_TargetToOrbit에서 추가 로직이 실행됩니다.
	if (!HasAuthority()) // 서버에서만 이 함수를 직접 호출해야 함. 클라이언트는 OnRep_TargetToOrbit을 통해 업데이트됨.
	{
		// 클라이언트에서 SetTargetActor가 불필요하게 호출되는 경우를 대비한 방어 코드
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: SetTargetActor called on client, which should be handled by OnRep_TargetToOrbit. Ignoring."));
		return;
	}

	TargetToOrbit = NewTarget;

	// 서버에서 대상 설정 시 Tick을 바로 활성화하여 관전 시작
	// 클라이언트에서는 OnRep_TargetToOrbit에서 처리하므로 여기서 IsLocallyControlled() 체크 필요
	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		SetActorTickEnabled(true);
		UpdateSpectatorPositionAndRotation(); // 초기 위치 설정
	}
	else if (!IsValid(TargetToOrbit)) // 서버에서 대상이 null로 설정될 경우 (예: 관전 종료 시)
	{
		SetActorTickEnabled(false);
	}
}

void APGSpectatorPawn::UpdateOrbitYawInput(float DeltaYaw)
{
	if (IsValid(TargetToOrbit))
	{
		CurrentOrbitYawAngle += DeltaYaw * RotationSpeed * GetWorld()->GetDeltaSeconds();
		CurrentOrbitYawAngle = FMath::Fmod(CurrentOrbitYawAngle, 360.0f);
		if (CurrentOrbitYawAngle < 0.0f)
		{
			CurrentOrbitYawAngle += 360.0f;
		}

		// Yaw가 변경되었으니 즉시 위치/회전 업데이트
		UpdateSpectatorPositionAndRotation();
		UE_LOG(LogTemp, Verbose, TEXT("SpectatorPawn: UpdateOrbitYawInput: CurrentOrbitYawAngle: %.2f"), CurrentOrbitYawAngle);
	}
}

void APGSpectatorPawn::UpdateSpectatorPositionAndRotation()
{
	if (!TargetToOrbit)
	{
		UE_LOG(LogTemp, Error, TEXT("SpectatorPawn: UpdateSpectatorPositionAndRotation: TargetToOrbit is null. Cannot update."));
		return;
	}

	FVector TargetLocation = TargetToOrbit->GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);

	// CurrentOrbitYawAngle과 CurrentOrbitDistance를 사용하여 NewLocation 계산 (기존과 동일)
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 80.0f);

	// SpectatorPawn의 위치는 직접 업데이트합니다. (기존과 동일)
	SetActorLocation(NewLocation);

	// NewLocation에서 TargetLocation을 바라보는 벡터를 구하여 회전을 계산합니다. (기존과 동일)
	FVector LookAtVector = TargetLocation - NewLocation;
	FRotator NewRotation = LookAtVector.Rotation();

	// SpectatorPawn 자체의 SetActorRotation 대신,
	// 이 SpectatorPawn을 현재 Possess하고 있는 PlayerController의 ControlRotation을 업데이트합니다.
	// SpectatorPawn의 카메라(뷰)는 이 ControlRotation을 따르게 됩니다.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(NewRotation);
	}
	else
	{
		// 만약 어떤 이유로 PlayerController에 의해 Possess되지 않았다면 (예: 전용 서버),
		// 또는 GetController()가 null을 반환한다면, 기존처럼 ActorRotation을 직접 설정하는 Fallback 로직입니다.
		// 하지만 클라이언트에서 플레이어가 SpectatorPawn을 Possess하는 상황이라면 이 else 블록은 실행되지 않을 것입니다.
		SetActorRotation(NewRotation);
	}

	if (GetWorld())
	{
		DrawDebugLine(GetWorld(), NewLocation, TargetLocation, FColor::Red, false, -1.f, 0, 5.f);
		DrawDebugDirectionalArrow(GetWorld(), NewLocation, NewLocation + GetActorForwardVector() * 100.f, 30.f, FColor::Green, false, -1.f, 0, 5.f);
		DrawDebugSphere(GetWorld(), TargetLocation, 20.f, 12, FColor::Blue, false, -1.f, 0, 5.f);
	}
}

void APGSpectatorPawn::OnRep_TargetToOrbit()
{
	UE_LOG(LogTemp, Log, TEXT("SpectatorPawn: OnRep_TargetToOrbit called. IsLocalPlayerController: %d, Target: %s"),
		IsLocallyControlled(), *GetNameSafe(TargetToOrbit));

	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		// 대상이 변경되었으므로 즉시 위치 및 회전 업데이트
		UpdateSpectatorPositionAndRotation(); // 초기 위치 설정
		// TargetToOrbit 유효할 시 Tick 활성화 
		SetActorTickEnabled(true);
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: OnRep_TargetToOrbit: Called SetActorTickEnabled(true). IsActorTickEnabled: %d"),
			IsActorTickEnabled());

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: OnRep_TargetToOrbit: Not locally controlled or TargetToOrbit is invalid."));
		// 유효한 대상이 없을 때 Tick 비활성화
		SetActorTickEnabled(true);

	}
}

//void APGSpectatorPawn::OnTargetCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
//{
//	// 이 함수는 ACharacter의 OnCharacterMovementUpdated 델리게이트가 호출될 때 실행됩니다.
//	// 클라이언트에서 시뮬레이트된 프록시의 움직임 업데이트에 반응하여 SpectatorPawn의 위치를 동기화합니다.
//	// Tick에서 이미 업데이트하고 있으므로, 이 델리게이트는 초기 동기화 또는 간헐적인 보정용으로 사용될 수 있습니다.
//	UE_LOG(LogTemp, Log, TEXT("SpectatorPawn: OnTargetCharacterMovementUpdated called. Target: %s. IsLocalControlled: %d"), *GetNameSafe(TargetToOrbit), IsLocallyControlled());
//	if (IsLocallyControlled())
//	{
//		UpdateSpectatorPositionAndRotation();
//	}
//
//}