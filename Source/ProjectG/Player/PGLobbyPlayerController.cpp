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

	//// if ClientTravel failed GI->HasClientTravelled == true
	//if (!HasAuthority())
	//{
	//	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	//	{
	//		if (GI->HasClientTravelled())
	//		{
	//			TravelCheck();
	//		}
	//	}
	//}
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

// if ClientTravel successed -> LobbyPlayerController destroy -> timer auto clear
void APGLobbyPlayerController::Client_StartTravelCheckTimer_Implementation()
{
	if (HasAuthority()) return;
	
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->MarkClientTravelled();
		GetWorldTimerManager().SetTimer(TravelCheckTimerHandle, this, &APGLobbyPlayerController::TravelCheck, 10.0f, false);
	}
}

// 1) ClientTravel failed -> LobbyPlayerController respawn -> Beginplay if(HasClientTravelled == true)->TravelCheck
// 2) ClientTravel failed -> LobbyPlayerController do not respawn -> call by TravelCheckTimer
void APGLobbyPlayerController::TravelCheck()
{
	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (GI->HasExceededRetryLimit())
		{
			// if Exceed ClientTravel retry count
			UE_LOG(LogTemp, Warning, TEXT("LobbyPlayerController: [%s] Exceed ClientTravel retry count. Destroy client session"), *GetNameSafe(this));
			GI->LeaveSessionAndReturnToLobby();
			return;
		}

		// if ClientTravel failed
		UE_LOG(LogTemp, Warning, TEXT("LobbyPlayerController: [%s] ClientTravel Failed. Force ClientTravel to sync."), *GetNameSafe(this));
		GI->MarkClientTravelled();
		GI->IncrementTravelRetryCount();

		GetWorldTimerManager().SetTimer(TravelCheckTimerHandle, this, &APGLobbyPlayerController::TravelCheck, 10.0f, false);

		ClientTravel("/Game/ProjectG/Levels/LV_PGMainLevel", ETravelType::TRAVEL_Absolute);
	}
}
