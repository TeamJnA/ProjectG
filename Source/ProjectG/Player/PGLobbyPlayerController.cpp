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
#include "UI/PGHUD.h"

APGLobbyPlayerController::APGLobbyPlayerController()
{

}

/*
* LobbyLevel ���� �� ���� �÷��̾��� ���� ������ ���¿� ���� �б�
* 1. ���� �÷��̾ ȣ��Ʈ�� ���
*	 ������ �����ϸ� GameInstance�� bIsHost������ true�� ���� -> bIsHost = true �� ���� ������ ������ ����
*    bIsHost�� ������ ������ ������ �����Ǿ� ȣ��Ʈ�� �׻� LobbyLevel���� Lobby ȯ�� ����
* 2. ���� �÷��̾ ȣ��Ʈ�� �ƴϰ� ���� ���� ���°� EGameState::MainMenu�� ���
*	 ������ ���� ����
*	 MainMenu ȯ�� ����
* 3. ���� �÷��̾ ȣ��Ʈ�� �ƴϰ� ���� ���� ���°� EGameState::Lobby�� ���
*	 ���ǿ� ������ Ŭ���̾�Ʈ�� ����
*    Lobby ȯ�� ����
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

	const UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());
	const APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GI || !GS)
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyPC::BeginPlay: GI or GS not valid"));
		return;
	}

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

void APGLobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// ESC
		EnhancedInputComponent->BindAction(ShowPauseMenuAction, ETriggerEvent::Started, this, &APGLobbyPlayerController::OnShowPauseMenu);
	}
}

/*
* Lobby �÷��̾� ĳ���� ���� �� ��û�� ��Ʈ�ѷ��� Poseess
* GameMode�� ��û�ϱ� ���� ServerRPC ���
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
* Lobby ���� ����
* HUD�� ���� ���� ��û
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
* MainMenu ���� ����
* HUD�� ���� ���� ��û
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
* MainMenu ��� ���÷���
* ������ ��ġ�ص� ī�޶� �þ� ���
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
