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
		GI->CheckIsTimerActive();
		GI->NotifyTravelSuccess();
	}
}

void APGPlayerController::NotifyStartTravel()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::NotifyStartTravel: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());
	MC_InitiateTravelTimer();
}

void APGPlayerController::MC_InitiateTravelTimer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::MC_InitClientTravelTimer: [%s] client travel timer start | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		GI->InitiateTravelTimer();
	}
}
