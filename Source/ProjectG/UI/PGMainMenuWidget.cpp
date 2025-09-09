// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMainMenuWidget.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

#include "UI/PGSessionSlotWidget.h"
#include "UI/PGConfirmWidget.h"
#include "UI/PGSettingMenuWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Player/PGLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UPGMainMenuWidget::AddSessionSlot(const FString& ServerName, int32 Index)
{
	if (!SessionListContainer || !SessionSlotWidgetClass) return;

	UPGSessionSlotWidget* slot = CreateWidget<UPGSessionSlotWidget>(this, SessionSlotWidgetClass);
	if (slot)
	{
		if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
		{
			slot->Setup(ServerName, Index, GI);
		}
		SessionListContainer->AddChild(slot);
	}
}

void UPGMainMenuWidget::ClearSessionList()
{
	if (SessionListContainer)
	{
		SessionListContainer->ClearChildren();
	}
}

void UPGMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedPC = UGameplayStatics::GetPlayerController(this, 0);

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnHostButtonClicked);
	}
	
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnJoinButtonClicked);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnOptionButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnExitButtonClicked);
	}

	if (OptionMenuCanvas_BackButton)
	{
		OptionMenuCanvas_BackButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnBackButtonClicked);
	}
	
	if (SessionListCanvas_BackButton)
	{
		SessionListCanvas_BackButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnBackButtonClicked);
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnRefreshButtonClicked);
	}

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->OnSessionsFound.AddUObject(this, &UPGMainMenuWidget::OnSessionsFound);
	}
}

void UPGMainMenuWidget::OnHostButtonClicked()
{
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnHostButtonClicked: Host button clicked"));
		GI->HostSession(NAME_GameSession, 4, false);
	}
}

void UPGMainMenuWidget::OnJoinButtonClicked()
{
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnJoinButtonClicked: Join button clicked"));
		if (WidgetSwitcher)
		{
			WidgetSwitcher->SetActiveWidgetIndex(2);
		}

		ClearSessionList();
		GI->FindSessions();
	}
}

void UPGMainMenuWidget::OnExitButtonClicked()
{
	if (ConfirmWidgetInstance && ConfirmWidgetInstance->IsInViewport())
	{
		return;
	}

	if (ConfirmWidgetClass)
	{
		ConfirmWidgetInstance = CreateWidget<UPGConfirmWidget>(this, ConfirmWidgetClass);
		ConfirmWidgetInstance->SetOwningPlayerController(CachedPC);
		if (ConfirmWidgetInstance)
		{
			ConfirmWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnExitButtonClicked: Exit Confirm pop-up added"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget::OnExitButtonClicked: ConfirmWidget class is null"));
	}

}

void UPGMainMenuWidget::OnOptionButtonClicked()
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(1);
	}
}

void UPGMainMenuWidget::OnRefreshButtonClicked()
{
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnRefreshButtonClicked: Refresh Button Clicked. Finding sessions..."));
		ClearSessionList();
		GI->FindSessions();
	}
}

void UPGMainMenuWidget::OnBackButtonClicked()
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(0);
	}
}

void UPGMainMenuWidget::OnSessionsFound(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	ClearSessionList();

	if (SessionResults.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnSessionsFound: No sessions found"));
		return;
	}

	for (int32 i = 0; i < SessionResults.Num(); ++i)
	{
		const FOnlineSessionSearchResult& Result = SessionResults[i];

		FString ServerName = Result.Session.OwningUserName;
		FString CustomServerName;
		CustomServerName = FString::Printf(TEXT("%s Session"), *ServerName);

		AddSessionSlot(CustomServerName, i);
	}
}
