// Fill out your copyright notice in the Description page of Project Settings.


#include "PGSessionSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

void UPGSessionSlotWidget::Setup(const FString& SessionName, int32 SessionIndex, UPGAdvancedFriendsGameInstance* GI)
{
	Index = SessionIndex;
	GameInstanceRef = GI;

	if (SessionNameText)
	{
		SessionNameText->SetText(FText::FromString(SessionName));
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UPGSessionSlotWidget::OnJoinClicked);
	}
}

void UPGSessionSlotWidget::OnJoinClicked()
{
	if (UPGAdvancedFriendsGameInstance* gi = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		gi->JoinSession(Index);
	}
}
