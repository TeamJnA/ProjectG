// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"
#include "Player/PGPlayerController.h"

void APGGameState::Multicast_MapGenerationComplete_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_MapGenerationComplete | HasAuthority = %d"), HasAuthority());
	OnMapGenerationComplete.Broadcast();
}

void APGGameState::Multicast_ClientTravelComplete_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_ClientTravel | HasAuthority = %d"), HasAuthority());
	OnClientTravelComplete.Broadcast();
}

void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
}

void APGGameState::NotifyClientTravelComplete(APGPlayerController* PC)
{
	if (ClientTravelCompletedPlayersQueue.Contains(PC)) return;

	if (bGameModeReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState: Gamemode is ready -> Broadcast to GameMode"), *PC->GetName());
		UE_LOG(LogTemp, Warning, TEXT("GameState: [%s] is ready | HasAuthority = %d"), *PC->GetName(), HasAuthority());
		Multicast_ClientTravelComplete();
		ClientTravelCompletedPlayersQueue.Add(PC);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState: Gamemode is not ready -> queue PC [%s]"), *PC->GetName());
		ClientTravelCompletedPlayersQueue.Add(PC);
	}
}

void APGGameState::NotifyGameModeReady()
{
	// if already notified -> return
	if (bGameModeReady) return;

	bGameModeReady = true;

	for (APlayerController* PC : ClientTravelCompletedPlayersQueue)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState: Gamemoed spawned. Broadcast delayed PC [%s] to Gamemode."), *PC->GetName());
		UE_LOG(LogTemp, Warning, TEXT("GameState: [%s] is ready | HasAuthority = %d"), *PC->GetName(), HasAuthority());
		Multicast_ClientTravelComplete();
	}

	// ClientTravelCompletedPlayersQueue.Empty();
}
