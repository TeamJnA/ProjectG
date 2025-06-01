// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGLobbyGameMode.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Kismet/GameplayStatics.h"
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
		if (APGGameState* GS = Cast<APGGameState>(GameState))
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel check logic [%s]"), *GS->GetName());
			GS->NotifyStartTravel();
		}

		world->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel"));
			GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
		}));
	}
}
