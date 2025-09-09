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

void APGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyPC::BeginPlay: %s"), *GetNameSafe(this));

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 1);
	}

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GI || !GS)
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyPC::BeginPlay: GI or GS not valid"));
		return;
	}

	if (GI->IsHost())
	{
		Server_SpawnAndPossessPlayer();
		SetupLobbyUI();
		GS->SetCurrentGameState(EGameState::Lobby);
	}
	else
	{
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
* on server
* spawn lobby character and possess it to requested PC
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
		GI->LeaveSessionAndReturnToLobby();
	}
}

/*
* On each client
* setup own lobby ui
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

void APGLobbyPlayerController::SetupMainMenuView()
{
	if (LobbyCameraClass)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), LobbyCameraClass, FoundActors);

		ACameraActor* TargetCamera = nullptr;
		if (FoundActors.Num() > 0)
		{
			TargetCamera = Cast<ACameraActor>(FoundActors[0]);
			UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: %s ACameraActor %s found"), *GetName(), *TargetCamera->GetName());
		}

		if (TargetCamera)
		{
			SetViewTarget(TargetCamera);
			UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: %s set view target %s"), *GetName(), *TargetCamera->GetName());
		}
	}
}

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
