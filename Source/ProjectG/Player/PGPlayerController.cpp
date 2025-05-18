// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"
#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"


void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

	bShowMouseCursor = false;

	UE_LOG(LogTemp, Warning, TEXT("PlayerController %s BeginPlay"), *GetNameSafe(this));

	if (HasAuthority()) return;
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		if (GI->HasClientTravelled())
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController %s: Detected previous ClientTravel. ReportClientTravel to Server"), *GetNameSafe(this));
			GI->ResetClientTravelFlag();
			Server_ReportClientTravel();
		}
	}
}

void APGPlayerController::Client_CheckLevelSync_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerController_Client: CheckLevelSync | HasAuthority %d"), HasAuthority());
	FString currentMap = GetWorld()->GetMapName();
	FString targetMap = "LV_PGMainLevel";

	if (currentMap.Contains(targetMap))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client map match"));
		UE_LOG(LogTemp, Warning, TEXT("PlayerController: ReportClientReady to server"));
		Server_ReportClientReady();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client map mismatch. Force ClientTravel to sync."));
		if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
		{
			GI->MarkClientTravelled();
			UE_LOG(LogTemp, Warning, TEXT("[%s] bDidClientTravel: %d"), *GetNameSafe(this), GI->HasClientTravelled());
		}
		ClientTravel("/Game/ProjectG/Levels/LV_PGMainLevel", ETravelType::TRAVEL_Absolute);
	}
}

void APGPlayerController::Server_ReportClientReady_Implementation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController_Server: Recieved client request ClientReady | HasAuthority %d"), HasAuthority());
		if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
		{
			GS->NotifyClientReady(this);
		}
	}
}

void APGPlayerController::Server_ReportClientTravel_Implementation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController_Server: Recieved client request ClientTravel | HasAuthority %d"), HasAuthority());
		if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
		{
			GS->NotifyClientTravel();
		}
	}
}
