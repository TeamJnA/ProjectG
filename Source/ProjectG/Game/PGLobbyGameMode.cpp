// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGLobbyGameMode.h"
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
		world->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen");
	}
}
