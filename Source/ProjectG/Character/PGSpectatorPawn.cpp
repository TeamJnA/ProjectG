// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

#include "Net/UnrealNetwork.h"

APGSpectatorPawn::APGSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	// ��Ʈ��ũ ���ø����̼� ����
	bReplicates = true;
	bAlwaysRelevant = true; // �׻� Ŭ���̾�Ʈ���� ���ü��� �ֵ��� ���� (���� ����)

	RotationSpeed = 90.0f; // �⺻ ȸ�� �ӵ�
	TargetToOrbit = nullptr;

	// ���� �߰��� �κ�
	CurrentOrbitDistance = 100.0f; // �����ڿ� ��� ���� �ʱ� �Ÿ� (���ϴ� ������ ����)
	CurrentOrbitYawAngle = 0.0f; // �ʱ� Yaw ����
}

void APGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APGSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Ŭ���̾�Ʈ���� Possess�� ���, Tick���� ��ġ/ȸ���� ������Ʈ�մϴ�.
	// �̰��� �ε巯�� ���� �������� ���� �ٽ��Դϴ�.
	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		UpdateSpectatorPositionAndRotation();
		//UE_LOG(LogTemp, Verbose, TEXT("SpectatorPawn: Tick is firing and updating position."));
	}
}

void APGSpectatorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSpectatorPawn, TargetToOrbit); // TargetToOrbit�� �����ϵ��� ����

}

void APGSpectatorPawn::SetTargetActor(AActor* NewTarget)
{	
	// �� �Լ��� �ַ� �������� TargetToOrbit ���� �����ϴ� �뵵�� ���˴ϴ�.
	// TargetToOrbit�� ReplicatedUsing=OnRep_TargetToOrbit�̹Ƿ�, Ŭ���̾�Ʈ������ OnRep_TargetToOrbit���� �߰� ������ ����˴ϴ�.
	if (!HasAuthority()) // ���������� �� �Լ��� ���� ȣ���ؾ� ��. Ŭ���̾�Ʈ�� OnRep_TargetToOrbit�� ���� ������Ʈ��.
	{
		// Ŭ���̾�Ʈ���� SetTargetActor�� ���ʿ��ϰ� ȣ��Ǵ� ��츦 ����� ��� �ڵ�
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: SetTargetActor called on client, which should be handled by OnRep_TargetToOrbit. Ignoring."));
		return;
	}

	TargetToOrbit = NewTarget;

	// �������� ��� ���� �� Tick�� �ٷ� Ȱ��ȭ�Ͽ� ���� ����
	// Ŭ���̾�Ʈ������ OnRep_TargetToOrbit���� ó���ϹǷ� ���⼭ IsLocallyControlled() üũ �ʿ�
	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		SetActorTickEnabled(true);
		UpdateSpectatorPositionAndRotation(); // �ʱ� ��ġ ����
	}
	else if (!IsValid(TargetToOrbit)) // �������� ����� null�� ������ ��� (��: ���� ���� ��)
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

		// Yaw�� ����Ǿ����� ��� ��ġ/ȸ�� ������Ʈ
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

	// CurrentOrbitYawAngle�� CurrentOrbitDistance�� ����Ͽ� NewLocation ��� (������ ����)
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 80.0f);

	// SpectatorPawn�� ��ġ�� ���� ������Ʈ�մϴ�. (������ ����)
	SetActorLocation(NewLocation);

	// NewLocation���� TargetLocation�� �ٶ󺸴� ���͸� ���Ͽ� ȸ���� ����մϴ�. (������ ����)
	FVector LookAtVector = TargetLocation - NewLocation;
	FRotator NewRotation = LookAtVector.Rotation();

	// SpectatorPawn ��ü�� SetActorRotation ���,
	// �� SpectatorPawn�� ���� Possess�ϰ� �ִ� PlayerController�� ControlRotation�� ������Ʈ�մϴ�.
	// SpectatorPawn�� ī�޶�(��)�� �� ControlRotation�� ������ �˴ϴ�.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(NewRotation);
	}
	else
	{
		// ���� � ������ PlayerController�� ���� Possess���� �ʾҴٸ� (��: ���� ����),
		// �Ǵ� GetController()�� null�� ��ȯ�Ѵٸ�, ����ó�� ActorRotation�� ���� �����ϴ� Fallback �����Դϴ�.
		// ������ Ŭ���̾�Ʈ���� �÷��̾ SpectatorPawn�� Possess�ϴ� ��Ȳ�̶�� �� else ����� ������� ���� ���Դϴ�.
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
		// ����� ����Ǿ����Ƿ� ��� ��ġ �� ȸ�� ������Ʈ
		UpdateSpectatorPositionAndRotation(); // �ʱ� ��ġ ����
		// TargetToOrbit ��ȿ�� �� Tick Ȱ��ȭ 
		SetActorTickEnabled(true);
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: OnRep_TargetToOrbit: Called SetActorTickEnabled(true). IsActorTickEnabled: %d"),
			IsActorTickEnabled());

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: OnRep_TargetToOrbit: Not locally controlled or TargetToOrbit is invalid."));
		// ��ȿ�� ����� ���� �� Tick ��Ȱ��ȭ
		SetActorTickEnabled(true);

	}
}

//void APGSpectatorPawn::OnTargetCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
//{
//	// �� �Լ��� ACharacter�� OnCharacterMovementUpdated ��������Ʈ�� ȣ��� �� ����˴ϴ�.
//	// Ŭ���̾�Ʈ���� �ùķ���Ʈ�� ���Ͻ��� ������ ������Ʈ�� �����Ͽ� SpectatorPawn�� ��ġ�� ����ȭ�մϴ�.
//	// Tick���� �̹� ������Ʈ�ϰ� �����Ƿ�, �� ��������Ʈ�� �ʱ� ����ȭ �Ǵ� �������� ���������� ���� �� �ֽ��ϴ�.
//	UE_LOG(LogTemp, Log, TEXT("SpectatorPawn: OnTargetCharacterMovementUpdated called. Target: %s. IsLocalControlled: %d"), *GetNameSafe(TargetToOrbit), IsLocallyControlled());
//	if (IsLocallyControlled())
//	{
//		UpdateSpectatorPositionAndRotation();
//	}
//
//}