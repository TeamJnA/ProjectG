// Fill out your copyright notice in the Description page of Project Settings.


#include "PGSessionSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "OnlineSessionSettings.h"

/*
* 세션 슬롯 Setup 구현부
*/
void UPGSessionSlotWidget::Setup(const FOnlineSessionSearchResult& SearchResult, int32 SessionIndex, UPGAdvancedFriendsGameInstance* GI)
{
	Index = SessionIndex;
	GameInstanceRef = GI;

	if (SessionNameText)
	{
		FString SessionName = FString::Printf(TEXT("%s Session"), *SearchResult.Session.OwningUserName);
		SessionNameText->SetText(FText::FromString(SessionName));
	}

	if (PlayerCountText)
	{
		const FOnlineSessionSettings& SessionSettings = SearchResult.Session.SessionSettings;
		const int32 MaxPlayers = SessionSettings.NumPublicConnections;
		int32 CurrentPlayers = 0;

		if (SessionSettings.Settings.Contains(SESSION_KEY_CURRENT_PLAYERS))
		{
			SessionSettings.Settings[SESSION_KEY_CURRENT_PLAYERS].Data.GetValue(CurrentPlayers);
		}
		else
		{
			CurrentPlayers = MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			UE_LOG(LogTemp, Warning, TEXT("SessionSlotWidget::Setup: Could not find CURRENT_PLAYERS key. Using fallback logic."));
		}

		FText PlayerCount = FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers));
		PlayerCountText->SetText(PlayerCount);
	}

	if (PingText)
	{
		FText Ping = FText::FromString(FString::Printf(TEXT("%d ms"), SearchResult.PingInMs));
		PingText->SetText(Ping);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UPGSessionSlotWidget::OnJoinClicked);
	}
}

/*
* 선택한 세션에 참가 요청
*/
void UPGSessionSlotWidget::OnJoinClicked()
{
	if (GameInstanceRef)
	{
		UE_LOG(LogTemp, Log, TEXT("SessionSlotWidget::OnJoinClicked: Session Slot Join Button Clicked for index %d."), Index);
		GameInstanceRef->JoinFoundSession(Index);
	}
}
