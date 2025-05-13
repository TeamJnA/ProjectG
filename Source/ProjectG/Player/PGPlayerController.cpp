// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"
#include "Game/PGGameState.h"


void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("PlayerController %s BeginPlay"), *GetName());

	// only client
	// if (HasAuthority()) return;
		
	/*
	if (GetWorld())
	{
		APGGameState* gs = GetWorld()->GetGameState<APGGameState>();
		if (gs)
		{
			gs->OnMapGenerationComplete.AddDynamic(this, &APGPlayerController::HandleMapGenerationComplete);
		}
	}
	*/
}

/*
void APGPlayerController::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerController: Map generation complete received on client | HasAuthority %d"), HasAuthority());

	GetWorld()->GetTimerManager().SetTimer(
		TravelCheckHandle,
		this,
		&APGPlayerController::CheckLevelSync,
		1.0f,
		false
	);
}
*/

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
	if (APGGameState* gs = GetWorld()->GetGameState<APGGameState>())
	{
		gs->NotifyClientReady(this);
	}
}

/*
void APGPlayerController::CheckLevelSync()
{
	FString currentMap = GetWorld()->GetMapName();
	FString TargetMap = "LV_PGMainLevel";

	if (!currentMap.Contains(TargetMap))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client map mismatch. Force ClientTravel to sync."));
		ClientTravel("/Game/ProjectG/Levels/LV_PGMainLevel.LV_PGMainLevel", ETravelType::TRAVEL_Absolute);
		// Need implementation after ClientTravel 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client map match"));
		UE_LOG(LogTemp, Warning, TEXT("PlayerController_Client: Send server request SpawnComplete | HasAuthority %d"), HasAuthority());
		Server_RequestSpawnComplete();
	}
}
*/