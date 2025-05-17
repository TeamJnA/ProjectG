// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/PGLobbyWidget.h"
#include "PGPlayerState.h"
#include "Game/PGLobbyGameMode.h"

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

			FInputModeUIOnly inputMode;
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
