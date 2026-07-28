// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGSessionSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Type/PGDifficultyTypes.h"


void UPGSessionSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddUniqueDynamic(this, &UPGSessionSlotWidget::OnJoinClicked);
	}
}

/*
* 세션 슬롯 Setup 구현부
*/
void UPGSessionSlotWidget::Setup(const FOnlineSessionSearchResult& SearchResult, int32 SessionIndex, UPGAdvancedFriendsGameInstance* GI)
{
	Index = SessionIndex;
	const FOnlineSessionSettings& SessionSettings = SearchResult.Session.SessionSettings;

	if (SessionNameText)
	{
		FString SessionName;
		if (!SessionSettings.Get(SESSION_KEY_SESSION_NAME, SessionName) || SessionName.IsEmpty())
		{
			SessionName = FString::Printf(TEXT("%s's Session"), *SearchResult.Session.OwningUserName);
		}
		SessionNameText->SetText(FText::FromString(SessionName));
	}

	if (PlayerCountText)
	{
		const int32 MaxPlayers = SessionSettings.NumPublicConnections;
		int32 CurrentPlayers = 0;

		if (!SessionSettings.Get(SESSION_KEY_CURRENT_PLAYERS, CurrentPlayers))
		{
			CurrentPlayers = MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			UE_LOG(LogTemp, Warning, TEXT("SessionSlotWidget::Setup: Could not find CURRENT_PLAYERS key. Using fallback logic."));
		}

		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers)));
	}

	if (DifficultyText)
	{
		int32 DiffValue = (int32)EPGDifficulty::Normal; // default: Normal
		SessionSettings.Get(SESSION_KEY_DIFFICULTY, DiffValue); // 키가 없으면 값 유지 -> Normal

		const EPGDifficulty Diff = (EPGDifficulty)DiffValue;
		const FString DiffStr = (Diff == EPGDifficulty::Hard) ? TEXT("HARD") : TEXT("NORMAL");
		DifficultyText->SetText(FText::FromString(DiffStr));
	}

	if (PingText)
	{
		PingText->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), SearchResult.PingInMs)));
	}
}

/*
* 선택한 세션에 참가 요청
*/
void UPGSessionSlotWidget::OnJoinClicked()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SessionSlotWidget::OnJoinClicked: Session Slot Join Button Clicked for index %d."), Index);
	GI->JoinFoundSession(Index);
}
