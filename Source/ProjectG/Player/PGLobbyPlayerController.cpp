// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"

#include "UI/PGMainMenuWidget.h"
#include "UI/PGLobbyWidget.h"

#include "PGPlayerState.h"
#include "Game/PGGameState.h"
#include "Game/PGLobbyGameMode.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

APGLobbyPlayerController::APGLobbyPlayerController()
{

}

UPGMainMenuWidget* APGLobbyPlayerController::GetMainMenuWidget() const
{
	return MainMenuWidgetInstance;
}

void APGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("LobbyPC::BeginPlay: %s"), *GetNameSafe(this));

		UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());
		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();

		if (GI)
		{
			if (GI->IsHost())
			{
				Server_SpawnAndPossessPlayer();
				SetupLobbyUI();
				if (GS)
				{
					GS->SetCurrentGameState(EGameState::Lobby);
				}
			}
			else
			{
				if (GS)
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
				else
				{
					UE_LOG(LogTemp, Error, TEXT("LobbyPC::BeginPlay: GameState not found"));
				}

			}
		}
	}
}

/*
* on server
* spawn lobby character and possess it to requested PC
*/
void APGLobbyPlayerController::Server_SpawnAndPossessPlayer_Implementation()
{
	// Create lobby character & possess
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world"));
	}

	if (!LobbyCharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("NO Character class"));
	}
	if (World && LobbyCharacterClass)
	{
		// 나중에 스폰 포인트 4개 만들어서 거기다가 스폰
		FVector SpawnLoc = FVector(920.0f, 0.0f, 100.0f);
		FRotator SpawnRot = FRotator(0.0f, 0.0f, 0.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ACharacter* LobbyCharacter = World->SpawnActor<ACharacter>(LobbyCharacterClass, SpawnLoc, SpawnRot, SpawnParams);
		if (LobbyCharacter)
		{
			//LobbyCharacter->FinishSpawning(FTransform(SpawnRot, SpawnLoc));
			Possess(LobbyCharacter);
			UE_LOG(LogTemp, Log, TEXT("LobbyPC::Server_SpawnAndPossessPlayer: Entered lobby state. Character spawned & possessed"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn Character"));

		}
	}
}

void APGLobbyPlayerController::SetReady()
{
	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (PS)
	{
		PS->ServerSetReady(true);
	}

	if (HasAuthority())
	{
		if (GetWorld()->GetAuthGameMode())
		{
			Cast<APGLobbyGameMode>(GetWorld()->GetAuthGameMode())->CheckAllPlayersReady();
		}
	}
}

/*
* On each client
* setup own lobby ui
*/
void APGLobbyPlayerController::SetupLobbyUI()
{
	// 1) Remove main menu widget
	if (MainMenuWidgetInstance && MainMenuWidgetInstance->IsInViewport())
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	// 2) Create lobby widget and add to viewport
	if (LobbyWidgetClass)
	{
		LobbyWidgetInstance = CreateWidget<UPGLobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidgetInstance)
		{
			LobbyWidgetInstance->AddToViewport();
			LobbyWidgetInstance->Init(this);

			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);

			bShowMouseCursor = true;

			UE_LOG(LogTemp, Log, TEXT("LobbyPC::SetupLobbyUI: Lobby widget add successed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("LobbyPC::SetupLobbyUI: Lobby widget add failed"));
	}
}

void APGLobbyPlayerController::SetupMainMenuUI()
{
	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UPGMainMenuWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			MainMenuWidgetInstance->SetIsFocusable(true);
			bShowMouseCursor = true;

			FInputModeGameAndUI inputMode;
			inputMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
			inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(inputMode);
		}
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
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: No ACameraActor found"));
		}

		if (TargetCamera)
		{
			SetViewTarget(TargetCamera);
			UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: %s set view target %s"), *GetName(), *TargetCamera->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: Failed to set view target (No valid camera actor)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyPC::BeginPlay: LobbyCamera is not set in PC Defaults."));
	}
}
