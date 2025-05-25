// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGLobbyGameMode.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerState.h"
#include "PGGameState.h"

APGLobbyGameMode::APGLobbyGameMode()
{
	PlayerControllerClass = APGLobbyPlayerController::StaticClass();
	PlayerStateClass = APGPlayerState::StaticClass();
	GameStateClass = APGGameState::StaticClass();

	bUseSeamlessTravel = true;
}

void APGLobbyGameMode::CheckAllPlayersReady()
{
	for (APlayerState* PS : GameState->PlayerArray)
	{
		APGPlayerState* myPS = Cast<APGPlayerState>(PS);
		if (!myPS || !myPS->bIsReady) return;
	}

	StartGame();
}

void APGLobbyGameMode::StartGame()
{
	UWorld* world = GetWorld();
	if (world)
	{
		if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(world->GetGameInstance()))
		{
			GI->SetExpectedPlayerCount(GameState->PlayerArray.Num());
		}
		if (GameState && GameState->PlayerArray.Num() > 0)
		{
			if (APGLobbyPlayerController* PC = Cast<APGLobbyPlayerController>(GameState->PlayerArray[0]->GetOwner()))
			{
				PC->Client_StartTravelCheckLogic();
			}
		}

		world->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}
}
