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

	// ��Ʈ��ũ ���ø����̼� ����
	bReplicates = true;
	bAlwaysRelevant = true; // �׻� Ŭ���̾�Ʈ���� ���ü��� �ֵ��� ���� (���� ����)

	// Get input actions
	ConstructorHelpers::FObjectFinder<UInputAction> OrbitYawActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_OrbitYaw.IA_OrbitYaw"));
	if (OrbitYawActionObj.Succeeded())
	{
		OrbitYawAction = OrbitYawActionObj.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> SpectateNextActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_SpectateNext.IA_SpectateNext"));
	if (SpectateNextActionObj.Succeeded())
	{
		SpectateNextAction = SpectateNextActionObj.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> SpectatePrevActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_SpectatePrev.IA_SpectatePrev"));
	if (SpectatePrevActionObj.Succeeded())
	{
		SpectatePrevAction = SpectatePrevActionObj.Object;
	}

	RotationSpeed = 90.0f; // �⺻ ȸ�� �ӵ�
	TargetToOrbit = nullptr;

	// ���� �߰��� �κ�
	CurrentOrbitDistance = 150.0f; // �����ڿ� ��� ���� �ʱ� �Ÿ� (���ϴ� ������ ����)
	CurrentOrbitYawAngle = 0.0f; // �ʱ� Yaw ����
}

bool APGSpectatorPawn::InitCachedAllPlayableCharacters(const APGPlayerCharacter* PrevPGCharacter)
{
	if (!PrevPGCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find PrevPGCharacter in APGPlayerController::Server_EnterSpectatorMode"));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World) 
		return false;

	if (!IsSpectateTargetCached)
	{
		for (TActorIterator<APGPlayerCharacter> It(World); It; ++It)
		{
			APGPlayerCharacter* CurrentChar = *It;

			if (CurrentChar && CurrentChar != PrevPGCharacter)
			{
				CachedAllPlayableCharacters.Add(CurrentChar);
			}
		}

		IsSpectateTargetCached = true;
	}

	if (CachedAllPlayableCharacters.IsEmpty())
	{
		return false;
	}
	return true;
}

void APGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APGSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Ŭ���̾�Ʈ���� Possess�� ���, Tick���� ��ġ/ȸ���� ������Ʈ
	if (IsLocallyControlled() && IsValid(TargetToOrbit))
	{
		UpdateSpectatorPositionAndRotation();
		//UE_LOG(LogTemp, Verbose, TEXT("SpectatorPawn: Tick is firing and updating position."));
	}
}

void APGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// up, down
		EnhancedInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &APGSpectatorPawn::OnSpectateNext);
		EnhancedInputComponent->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &APGSpectatorPawn::OnSpectatePrev);
		// left, right
		EnhancedInputComponent->BindAction(OrbitYawAction, ETriggerEvent::Triggered, this, &APGSpectatorPawn::OnOrbitYaw);
	}
}

void APGSpectatorPawn::OnSpectateNext(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	Server_SetSpectateTarget(true);
	UE_LOG(LogTemp, Log, TEXT("PC::OnSpectateNext: Client requested next spectate target."));
}

void APGSpectatorPawn::OnSpectatePrev(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	Server_SetSpectateTarget(false);
	UE_LOG(LogTemp, Log, TEXT("PC::OnSpectatePrev: Client requested previous spectate target."));
}

void APGSpectatorPawn::Server_SetSpectateTarget_Implementation(bool bNext)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// ���� ���� ����� �ִ� ��� �ش� ����� index ����
	int32 CurrentTargetIndex = INDEX_NONE;
	if (IsValid(SpectateTargetCharacter))
	{
		CurrentTargetIndex = CachedAllPlayableCharacters.IndexOfByKey(SpectateTargetCharacter);
	}

	// ���ο� ������� index
	int32 NewTargetIndex = 0;

	// ���� ���� ���� ���, �Է¿� ���� ���� ��� ���� Ȥ�� ���� ĳ������ �ε��� ����
	// ���� ������ �����ϴ� ���, �迭���� ���� ����� ã�� �ʰ�, �迭�� 0�� ĳ���� ����
	if (CurrentTargetIndex != INDEX_NONE)
	{
		if (bNext)
		{
			NewTargetIndex = (CurrentTargetIndex + 1) % CachedAllPlayableCharacters.Num();
		}
		else
		{
			NewTargetIndex = (CurrentTargetIndex - 1 + CachedAllPlayableCharacters.Num()) % CachedAllPlayableCharacters.Num();
		}
	}

	APGPlayerCharacter* NewTargetCandidate = CachedAllPlayableCharacters[NewTargetIndex];

	// ���ο� ����� ���� ���� ������ �ٸ� ��쿡�� ������Ʈ
	// ���ο� ����� ���� ���� ���� ��� => ���� ������ ĳ���Ͱ� ���ʿ� �� �� �ۿ� ������ ��� ������Ʈ x
	if (NewTargetCandidate != SpectateTargetCharacter)
	{
		SpectateTargetCharacter = NewTargetCandidate;
		UE_LOG(LogTemp, Log, TEXT("PC::Server_ChangeSpectateTarget_Implementation: Changed spectate target to: %s"), *GetNameSafe(SpectateTargetCharacter));

		if (IsValid(SpectateTargetCharacter))
		{
			SetTargetActor(SpectateTargetCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC::Server_ChangeSpectateTarget_Implementation: ControlledSpectator or SpectateTargetCharacter invalid after target change."));
		}
	}
	else
	{
		// �� �޽����� AllPlayableCharacters.Num() == 1 �� ���� ��� (���� ������ ĳ���Ͱ� ���ʿ� �� �� �ۿ� ������ ���)
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_ChangeSpectateTarget_Implementation: Spectate target remains the same (no other valid unique target)."));
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

void APGSpectatorPawn::OnOrbitYaw(const FInputActionValue& Value)
{
	// Ŭ���̾�Ʈ���� �Է� ó��
	if (!IsLocallyControlled()) return;

	float AxisValue = Value.Get<float>();

	// ���� �÷��̾� ��Ʈ�ѷ��� APGSpectatorPawn�� Possess�ϰ� ���� ���� ó��
	if (IsValid(TargetToOrbit))
	{
		CurrentOrbitYawAngle += AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
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

	// CurrentOrbitYawAngle�� CurrentOrbitDistance�� ����Ͽ� NewLocation ���
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 60.0f);

	// SpectatorPawn�� ��ġ�� ���� ������Ʈ
	SetActorLocation(NewLocation);

	// NewLocation���� TargetLocation�� �ٶ󺸴� ���͸� ���Ͽ� ȸ���� ���
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
