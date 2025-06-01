// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerController.h"

void APGGameState::Multicast_MapGenerationComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_MapGenerationComplete | HasAuthority = %d"), HasAuthority());
	OnMapGenerationComplete.Broadcast();
}

void APGGameState::MC_InitiateTravelTimer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("APGGameState::MC_InitiateTravelTimer: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		GI->InitiateTravelTimer();
	}	
}

void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
}

void APGGameState::NotifyStartTravel()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("APGGameState::NotifyStartTravel: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());
	MC_InitiateTravelTimer();
}
