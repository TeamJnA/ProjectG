// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"
#include "Player/PGPlayerController.h"

void APGGameState::Multicast_MapGenerationComplete_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_MapGenerationComplete | HasAuthority = %d"), HasAuthority());
	OnMapGenerationComplete.Broadcast();
}

void APGGameState::Multicast_SpawnComplete_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_SpawnComplete | HasAuthority = %d"), HasAuthority());
	OnSpawnComplete.Broadcast();
}

void APGGameState::Multicast_ClientTravel_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_ClientTravel | HasAuthority = %d"), HasAuthority());
	OnClientTravel.Broadcast();
}

void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
}

void APGGameState::NotifySpawnComplete()
{
	Multicast_SpawnComplete();
}

void APGGameState::NotifyClientTravel()
{
	Multicast_ClientTravel();
}

void APGGameState::NotifyClientReady(APGPlayerController* PC)
{
	if (ReadyPlayers.Contains(PC)) return;

	ReadyPlayers.Add(PC);

	UE_LOG(LogTemp, Warning, TEXT("Client ready: %s (%d / %d)"), *PC->GetName(), ReadyPlayers.Num(), PlayerArray.Num());

	if (ReadyPlayers.Num() == PlayerArray.Num())
	{
		NotifySpawnComplete();
	}
}
