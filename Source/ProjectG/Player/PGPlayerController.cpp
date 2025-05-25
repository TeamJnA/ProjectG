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
	// On travel retry success
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		if (GI->DidRetryClientTravel())
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::BeginPlay [%s] success retry client travel"), *GetNameSafe(this));
			GI->NotifyTravelSuccess();
		}
	}
}

void APGPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// On travel first try success
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::PostSeamlessTravel: [%s] travel success"), *GetNameSafe(this));
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		GI->NotifyTravelSuccess();
	}
}

void APGPlayerController::Client_InitiateTravelTimer_Implementation()
{
	if (HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::Client_InitClientTravelTimer: [%s] client travel timer start"), *GetNameSafe(this));
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		GI->InitiateTravelTimer();
	}
}
