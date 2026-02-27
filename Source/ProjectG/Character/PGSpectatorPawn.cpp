// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerController.h"
#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerState.h"

#include "Level/Exit/PGExitPointBase.h"

#include "EngineUtils.h" 
#include "Net/UnrealNetwork.h"
#include "Character/Component/PGVOIPTalker.h"

#include "Player/PGGameUserSettings.h"


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

void APGSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	TryInitVoiceSettings();
}

void APGSpectatorPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	TryInitVoiceSettings();
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
		// mouse movement input
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

	float Sensitivity = 1.0f;
	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		// 0~1 슬라이더 -> 0.2~2.0 배율
		Sensitivity = FMath::Lerp(0.2f, 2.0f, Settings->CameraSensitivity);
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	CurrentOrbitYawAngle += LookAxisVector.X * Sensitivity;
	CurrentOrbitPitchAngle -= LookAxisVector.Y * Sensitivity;
	CurrentOrbitPitchAngle = FMath::Clamp(CurrentOrbitPitchAngle, -49.0f, 69.0f);
}

/*
* 설정된 타겟과의 거리(150.0f)를 유지하며 트래킹/궤도 유지
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

	// Location -> PawnLocation 업데이트
	SetActorLocation(NewLocation);

	// Rotation -> ControlRotation 업데이트
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const FVector LookAtVector = TargetLocation - NewLocation;
		const FRotator NewRotation = LookAtVector.Rotation();
		PC->SetControlRotation(NewRotation);
	}
}

/*
* 관전 pawn의 관전 대상 설정
* 클라이언트에도 관전 대상 정보 레플리케이트
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
* 관전 대상 설정 시 Tick을 활성화하여 로컬에서 직접 추적
* 관전 대상 이동에 따른 회전, 대상과의 거리 업데이트
*/
void APGSpectatorPawn::OnRep_TargetToOrbit()
{
	if (IsLocallyControlled())
	{
		if (APGExitPointBase* ExitPoint = Cast<APGExitPointBase>(TargetToOrbit))
		{
			bCanOrbit = false;
			SetActorTickEnabled(false);
			SetActorLocation(ExitPoint->GetCameraLocation());
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->SetControlRotation(ExitPoint->GetCameraRoation());
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

// 로컬이 리모트에 대해 Voip를 구현해야함
void APGSpectatorPawn::TryInitVoiceSettings()
{
	FString NetModeStr = (GetNetMode() == NM_Client) ? TEXT("Client") : TEXT("Server");
	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] TryInitVoiceSettings triggered!"), *NetModeStr);

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] LocalPC not found! Retrying in 0.1s..."), *NetModeStr);
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGSpectatorPawn::TryInitVoiceSettings, 0.1f, false);
		return;
	}

	APGPlayerController* InGamePC = Cast<APGPlayerController>(LocalPC);
	APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(LocalPC);
	if (!InGamePC && !LobbyPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] Valid PC not found! Retrying in 0.1s..."), *NetModeStr);
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGSpectatorPawn::TryInitVoiceSettings, 0.1f, false);
		return;
	}

	// 로컬이면 Mute/Unmute 갱신
	// 리모트면 대상의 Voip, Mute/Unmute 갱신
	if (IsLocallyControlled())
	{
		if (InGamePC)
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] Local PGPC"));

			InGamePC->RefreshVoiceChannel();
		}
		else if (LobbyPC)
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] Local LobbyPC"));

			LobbyPC->RefreshVoiceChannel();
		}
	}
	else
	{
		APGPlayerState* TargetPS = GetPlayerState<APGPlayerState>();
		if (TargetPS && TargetPS->IsInactive())
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] TargetPS is Inactive. Aborting voice setup."));
			return;
		}

		if (!TargetPS || !TargetPS->GetUniqueId().IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] TargetPS not found! Retrying in 0.1s..."), *NetModeStr);
			FTimerHandle RetryHandle;
			GetWorldTimerManager().SetTimer(RetryHandle, this, &APGSpectatorPawn::TryInitVoiceSettings, 0.1f, false);
			return;
		}

		if (!VoipTalker)
		{
			VoipTalker = UPGVOIPTalker::CreateTalkerForPlayer(TargetPS);
			if (VoipTalker)
			{
				VoipTalker->RegisterWithPlayerState(TargetPS);
				VoipTalker->Settings.AttenuationSettings = nullptr;
				VoipTalker->Settings.ComponentToAttachTo = nullptr;
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] SUCCESS: Attached 3D Voice to %s (Owner: %s)"), *NetModeStr, *GetName(), *TargetPS->GetPlayerName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] Failed to create Talker. Retrying..."));
				FTimerHandle RetryHandle;
				GetWorldTimerManager().SetTimer(RetryHandle, this, &APGSpectatorPawn::TryInitVoiceSettings, 0.1f, false);
				return;
			}
		}

		if (InGamePC)
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] PGPC"));

			InGamePC->RefreshVoiceChannel();
		}
		else if (LobbyPC)
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] LobbyPC"));

			LobbyPC->RefreshVoiceChannel();
		}
	}
}