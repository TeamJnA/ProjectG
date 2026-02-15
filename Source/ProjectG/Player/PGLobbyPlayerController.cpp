// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGLobbyPlayerController.h"

#include "PGPlayerState.h"
#include "Game/PGGameState.h"
#include "Game/PGLobbyGameMode.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "Camera/CameraActor.h"
#include "UI/Manager/PGHUD.h"

#include "Net/VoiceConfig.h"
#include "EngineUtils.h"
#include "Character/PGPlayerCharacter.h"

APGLobbyPlayerController::APGLobbyPlayerController()
{

}

/*
* LobbyLevel 진입 시 로컬 플레이어의 상태 게임의 상태에 따라 분기
* 1. 로컬 플레이어가 호스트인 경우
*	 세션을 생성하며 GameInstance의 bIsHost변수를 true로 설정 -> bIsHost = true 인 경우는 세션을 생성한 상태
*    bIsHost는 세션을 종료할 때까지 유지되어 호스트는 항상 LobbyLevel에서 Lobby 환경 구축
* 2. 로컬 플레이어가 호스트가 아니고 현재 게임 상태가 EGameState::MainMenu인 경우
*	 세션이 없는 상태
*	 MainMenu 환경 구축
* 3. 로컬 플레이어가 호스트가 아니고 현재 게임 상태가 EGameState::Lobby인 경우
*	 세션에 참가한 클라이언트인 상태
*    Lobby 환경 구축
*/
void APGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 1);
	}

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GI || !GS)
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyPC::BeginPlay: GI or GS not valid"));
		return;
	}

	GI->ShowLoadingScreen();

	// GameInstance .h
	// bool IsHost() const { return bIsHost; }
	if (GI->IsHost())
	{
		Server_SpawnAndPossessPlayer();
		SetupLobbyUI();
	}
	else
	{
		// GameState .h
		// EGameState GetCurrentGameState() const { return CurrentGameState; }		
		switch (GS->GetCurrentGameState())
		{
		case EGameState::MainMenu:
			SetupMainMenuView();
			SetupMainMenuUI();
			break;

		case EGameState::Lobby:
			Server_SpawnAndPossessPlayer();
			SetupLobbyUI();
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: Current Game State is not MainMenu or Lobby"));
		}
	}
}

void APGLobbyPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// On travel first try success
	UE_LOG(LogTemp, Warning, TEXT("LobbyPC::PostSeamlessTravel: [%s] travel success"), *GetNameSafe(this));

	if (!IsLocalController())
	{
		return;
	}
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Client_PostSeamlessTravel_Implementation: No valid GI"));
		return;
	}
	GI->ShowLoadingScreen();
}

void APGLobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// ESC
		EnhancedInputComponent->BindAction(ShowPauseMenuAction, ETriggerEvent::Started, this, &APGLobbyPlayerController::OnShowPauseMenu);
	}
}

void APGLobbyPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (IsLocalController())
	{
		TryStartLobbyVoice();

		FTimerHandle HideTimer;
		GetWorld()->GetTimerManager().SetTimer(HideTimer, this, &APGLobbyPlayerController::HideLoadingScreenDelayed, 0.5f, false);
	}
}

void APGLobbyPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		TryStartLobbyVoice();

		FTimerHandle HideTimer;
		GetWorld()->GetTimerManager().SetTimer(HideTimer, this, &APGLobbyPlayerController::HideLoadingScreenDelayed, 0.5f, false);
	}
}

void APGLobbyPlayerController::TryStartLobbyVoice()
{
	if (GetPawn())
	{
		StartTalking();
		UE_LOG(LogTemp, Log, TEXT("Lobby Voice: Started Talking (Has Pawn)"));
	}
}

void APGLobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalController())
	{
		StopTalking();
	}
	Super::EndPlay(EndPlayReason);
}

void APGLobbyPlayerController::HideLoadingScreenDelayed()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyPC::HideLoadingScreenDelayed: No valid GI"));
		return;
	}
	GI->HideLoadingScreen();
}

/*
* Lobby 플레이어 캐릭터 생성 및 요청한 컨트롤러에 Poseess
* GameMode에 요청하기 위해 ServerRPC 사용
*/
void APGLobbyPlayerController::Server_SpawnAndPossessPlayer_Implementation()
{
	// Create lobby character & possess
	if (APGLobbyGameMode* LobbyGM = GetWorld()->GetAuthGameMode<APGLobbyGameMode>())
	{
		LobbyGM->SpawnAndPossessPlayer(this);
	}
}

void APGLobbyPlayerController::Client_ForceReturnToLobby_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyPC::Client_ForceReturnToLobby: Received command from host to leave session"));

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
}

/*
* Lobby 위젯 생성
* HUD에 위젯 생성 요청
*/
void APGLobbyPlayerController::SetupLobbyUI()
{
	if (!IsLocalController())
	{
		return;
	}

	if (APGHUD* HUD = GetHUD<APGHUD>())
	{
		HUD->InitLobbyWidget();
	}
}

/*
* MainMenu 위젯 생성
* HUD에 위젯 생성 요청
*/
void APGLobbyPlayerController::SetupMainMenuUI()
{
	if (!IsLocalController())
	{
		return;
	}

	if (APGHUD* HUD = GetHUD<APGHUD>())
	{
		HUD->InitMainMenuWidget();
	}

}

/*
* MainMenu 배경 디스플레이
* 레벨에 배치해둔 카메라 시야 사용
*/
void APGLobbyPlayerController::SetupMainMenuView()
{
	if (LobbyCameraClass)
	{
		if (ACameraActor* TargetCamera = Cast<ACameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), LobbyCameraClass)))
		{
			SetViewTarget(TargetCamera);
		}
	}
}

/*
* 
*/
void APGLobbyPlayerController::OnShowPauseMenu(const FInputActionValue& Value)
{
	if (!IsLocalController())
	{
		return;
	}

	if (APGHUD* HUD = GetHUD<APGHUD>())
	{
		HUD->InitPauseMenuWidget();
	}
}

void APGLobbyPlayerController::Client_ShowLoadingScreen_Implementation()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyPC::Client_ShowLoadingScreen: No valid gi"));
	}
	GI->ShowLoadingScreen();
}

void APGLobbyPlayerController::Client_StopVoiceAndCleanup_Implementation(ECleanupActionType ActionType)
{
	if (ActionType != ECleanupActionType::None)
	{
		if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
		{
			GI->ShowLoadingScreen();
		}
	}

	PerformCleanup();

	FTimerHandle CleanupDelayTimer;
	GetWorld()->GetTimerManager().SetTimer(CleanupDelayTimer, [this]()
	{
		if (IsValid(this))
		{
			Server_NotifyCleanupFinished();
		}
	}, 1.0f, false);
}

void APGLobbyPlayerController::PerformCleanup()
{
	StopTalking();

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface();
		if (VoiceInterface.IsValid() && PlayerState)
		{
			VoiceInterface->MuteRemoteTalker(0, *PlayerState->GetUniqueId(), true);
			//VoiceInterface->RemoveAllRemoteTalkers();
			VoiceInterface->ClearVoicePackets();
		}
	}
}

void APGLobbyPlayerController::Client_RestartVoice_Implementation()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface();
		if (VoiceInterface.IsValid() && PlayerState)
		{
			VoiceInterface->UnmuteRemoteTalker(0, *PlayerState->GetUniqueId(), true);
		}
	}

	StartTalking();

	UE_LOG(LogTemp, Log, TEXT("LobbyPC: Voice Restarted."));
}

void APGLobbyPlayerController::Client_ExecuteSoloAction_Implementation(ECleanupActionType ActionType)
{
	PerformSessionEndAction(ActionType);
}

void APGLobbyPlayerController::PerformSessionEndAction(ECleanupActionType ActionType)
{
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		if (ActionType == ECleanupActionType::Solo_ReturnToMainMenu || ActionType == ECleanupActionType::Mass_KickForDestroy)
		{
			UE_LOG(LogTemp, Log, TEXT("LobbyPC: Executing ReturnToMainMenu"));
			GI->LeaveSessionAndReturnToMainMenu();
		}
		else if (ActionType == ECleanupActionType::Solo_QuitToDesktop)
		{
			UE_LOG(LogTemp, Log, TEXT("LobbyPC: Executing QuitToDesktop"));
			UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
		}
	}
}

void APGLobbyPlayerController::Server_NotifyCleanupFinished_Implementation()
{
	if (APGLobbyGameMode* GM = Cast<APGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnPlayerCleanupFinished(this);
	}
}

void APGLobbyPlayerController::Server_RequestSoloLeave_Implementation(ECleanupActionType ActionType)
{
	if (APGLobbyGameMode* GM = Cast<APGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ProcessSoloLeaveRequest(this, ActionType);
	}
}

void APGLobbyPlayerController::Server_RequestSessionDestruction_Implementation(bool bServerQuit)
{
	if (APGLobbyGameMode* GM = Cast<APGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->RequestSessionDestruction(bServerQuit);
	}
}