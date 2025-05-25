// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"
#include "Player/PGPlayerController.h"

void APGGameState::Multicast_MapGenerationComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_MapGenerationComplete | HasAuthority = %d"), HasAuthority());
	OnMapGenerationComplete.Broadcast();
}

void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
}
