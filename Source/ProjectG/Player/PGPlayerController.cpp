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

	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController: [%s] BeginPlay"), *GetNameSafe(this));
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		if (GI->HasClientTravelled())
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPlayerController %s: Detected previous ClientTravel. Reset flag and retry count"), *GetNameSafe(this));
			GI->ResetClientTravelFlag();
			GI->ResetTravelRetryCount();
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController: [%s] Report Client Travel"), *GetNameSafe(this));
	Server_ReportClientTravelComplete();
}

void APGPlayerController::Server_ReportClientTravelComplete_Implementation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController_Server: Recieved client request ClientTravelComplete | HasAuthority %d"), HasAuthority());
		if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
		{
			GS->NotifyClientTravelComplete(this);
		}
	}
}
