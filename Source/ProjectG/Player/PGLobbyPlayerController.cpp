// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/PGLobbyWidget.h"
#include "PGPlayerState.h"
#include "Game/PGLobbyGameMode.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

APGLobbyPlayerController::APGLobbyPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/ProjectG/UI/WBP_LobbyWidget"));
	if (WidgetClass.Succeeded())
	{
		LobbyWidgetClass = WidgetClass.Class;
	}
}

UPGLobbyWidget* APGLobbyPlayerController::GetLobbyWidget() const
{
	return LobbyWidgetInstance;
}

void APGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidgetInstance = CreateWidget<UPGLobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidgetInstance)
		{
			LobbyWidgetInstance->AddToViewport();
			LobbyWidgetInstance->SetIsFocusable(true);
			bShowMouseCursor = true;

			FInputModeGameAndUI inputMode;
			inputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
			inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(inputMode);
		}
	}
}

void APGLobbyPlayerController::SetReady()
{
	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (PS)
	{
		PS->ServerSetReady(true);
	}

	if (HasAuthority())
	{
		if (GetWorld()->GetAuthGameMode())
		{
			Cast<APGLobbyGameMode>(GetWorld()->GetAuthGameMode())->CheckAllPlayersReady();
		}
	}
}

void APGLobbyPlayerController::NotifyStartTravel()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("LobbyPlayerController::NotifyStartTravel: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());
	MC_InitiateTravelTimer();
}

// if ClientTravel successed -> LobbyPlayerController destroy -> timer auto clear
void APGLobbyPlayerController::MC_InitiateTravelTimer_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("LobbyPlayerController::MC_InitiateTravelTimer: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->InitiateTravelTimer();
	}
}
