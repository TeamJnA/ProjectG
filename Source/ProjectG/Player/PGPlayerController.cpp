// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"
#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

	bShowMouseCursor = false;

	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController: [%s] BeginPlay"), *GetNameSafe(this));
}

void APGPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	
	if (!HasAuthority()) return;

	// On travel first try success
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::PostSeamlessTravel: [%s] travel success"), *GetNameSafe(this));
	Client_PostSeamlessTravel();
}

void APGPlayerController::Client_PostSeamlessTravel_Implementation()
{
	if (!IsLocalController()) return;

	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		if (GI->CheckIsTimerActive())
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::Client_PostSeamlessTravel: [%s] stop travel timer"), *GetNameSafe(this));
			GI->NotifyTravelSuccess();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::Client_PostSeamlessTravel: [%s] travel success but already timeout"), *GetNameSafe(this));
		}
	}
}

/*
* On host or client server travel failed
* call server RPC to redo server travel
* server PC Set travel failed flag on GM
*/
void APGPlayerController::NotifyTravelFailed()
{
	if (!IsLocalController()) return;

	UE_LOG(LogTemp, Warning, TEXT("PGPC::NotifyTravelFailed: [%s] Report travel failed to server."), *GetNameSafe(this));
	Server_ReportTravelFailed();
}

void APGPlayerController::Server_ReportTravelFailed_Implementation()
{
	if (!HasAuthority()) return;

	APGGameMode* GM = Cast<APGGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	UE_LOG(LogTemp, Warning, TEXT("PGPC::Server_ReportTravelFailed: Report travel failed to GM."));
	GM->SetIsTravelFailedExist();
}
