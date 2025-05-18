// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"
#include "Game/PGGameState.h"


void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

	bShowMouseCursor = false;

	UE_LOG(LogTemp, Warning, TEXT("PlayerController %s BeginPlay"), *GetName());
}

void APGPlayerController::Server_RequestSpawnComplete_Implementation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController_Server: Recieved client request SpawnComplete | HasAuthority %d"), HasAuthority());

		if (APGGameState* gs = GetWorld()->GetGameState<APGGameState>())
		{
			gs->NotifySpawnComplete();
		}
	}
}

// WIP
void APGPlayerController::Client_CheckLevelSync_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerController_Client: CheckLevelSync | HasAuthority %d"), HasAuthority());

	FString currentMap = GetWorld()->GetMapName();
	FString TargetMap = "LV_PGMainLevel";

	if (currentMap.Contains(TargetMap))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client map match"));
		Server_ReportClientReady();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client map mismatch. Force ClientTravel to sync."));
		ClientTravel("/Game/ProjectG/Levels/LV_PGMainLevel.LV_PGMainLevel", ETravelType::TRAVEL_Absolute);
		// Need implementation after ClientTravel 
	}
}

void APGPlayerController::Server_ReportClientReady_Implementation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController_Server: Recieved client request SpawnComplete | HasAuthority %d"), HasAuthority());

		if (APGGameState* gs = GetWorld()->GetGameState<APGGameState>())
		{
			gs->NotifyClientReady(this);
		}
	}
}
