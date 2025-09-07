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

	// 네트워크 리플리케이션 설정
	bReplicates = true;
	bAlwaysRelevant = true; // 항상 클라이언트에게 관련성이 있도록 설정 (선택 사항)

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

	RotationSpeed = 90.0f; // 기본 회전 속도
	TargetToOrbit = nullptr;

	// 새로 추가된 부분
	CurrentOrbitDistance = 150.0f; // 관전자와 대상 간의 초기 거리 (원하는 값으로 설정)
	CurrentOrbitYawAngle = 0.0f; // 초기 Yaw 각도
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
	// 클라이언트에서 Possess된 경우, Tick마다 위치/회전을 업데이트
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

	// 현재 관전 대상이 있는 경우 해당 대상의 index 저장
	int32 CurrentTargetIndex = INDEX_NONE;
	if (IsValid(SpectateTargetCharacter))
	{
		CurrentTargetIndex = CachedAllPlayableCharacters.IndexOfByKey(SpectateTargetCharacter);
	}

	// 새로운 관전대상 index
	int32 NewTargetIndex = 0;

	// 현재 관전 중인 경우, 입력에 따라 관전 대상 다음 혹은 이전 캐릭터의 인덱스 저장
	// 최초 관전을 시작하는 경우, 배열에서 관전 대상을 찾지 않고, 배열의 0번 캐릭터 관전
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

	// 새로운 대상이 현재 대상과 실제로 다를 경우에만 업데이트
	// 새로운 대상이 현재 대상과 같은 경우 => 관전 가능한 캐릭터가 애초에 한 명 밖에 없었을 경우 업데이트 x
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
		// 이 메시지는 AllPlayableCharacters.Num() == 1 일 때만 출력 (관전 가능한 캐릭터가 애초에 한 명 밖에 없었을 경우)
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_ChangeSpectateTarget_Implementation: Spectate target remains the same (no other valid unique target)."));
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

void APGSpectatorPawn::OnOrbitYaw(const FInputActionValue& Value)
{
	// 클라이언트에서 입력 처리
	if (!IsLocallyControlled()) return;

	float AxisValue = Value.Get<float>();

	// 현재 플레이어 컨트롤러가 APGSpectatorPawn을 Possess하고 있을 때만 처리
	if (IsValid(TargetToOrbit))
	{
		CurrentOrbitYawAngle += AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
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

	// CurrentOrbitYawAngle과 CurrentOrbitDistance를 사용하여 NewLocation 계산
	FVector RelativeVector = FRotator(0.0f, CurrentOrbitYawAngle, 0.0f).Vector() * CurrentOrbitDistance;
	FVector NewLocation = TargetLocation + RelativeVector + FVector(0.0f, 0.0f, 60.0f);

	// SpectatorPawn의 위치는 직접 업데이트
	SetActorLocation(NewLocation);

	// NewLocation에서 TargetLocation을 바라보는 벡터를 구하여 회전을 계산
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
