// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGPlayerEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Player/PGPlayerState.h"

#include "Type/CharacterTypes.h"

/*
* ���� �����͸� �������� �÷��̾� �̹���, �̸�, ���� ���÷���
* Ż��/��� ���� -> ScoreBoardWidget, FinalScoreBoardWidget
* ȣ��Ʈ ���� -> LobbyWidget
*/
void UPGPlayerEntryWidget::SetupEntry(const APGPlayerState* InPlayerState, UTexture2D* InAvatarTexture)
{
	UE_LOG(LogTemp, Log, TEXT("PlayerEntryWidget::SetupEntry: [%s]"), *InPlayerState->GetPlayerName());
	
	if (!InPlayerState)
	{
		return;
	}

	if (PlayerNameText) 
	{ 
		PlayerNameText->SetText(FText::FromString(InPlayerState->GetPlayerName()));
	}

	if (PlayerAvatar)
	{
		const bool bHasAvatar = (InAvatarTexture != nullptr);
		if (bHasAvatar)
		{
			PlayerAvatar->SetBrushFromTexture(InAvatarTexture);
		}
		PlayerAvatar->SetVisibility(bHasAvatar ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (StatusText)
	{
		int32 PlayerNameFontSize;

		int32 StatusFontSize;
		FText StatusMessage;
		FLinearColor StatusColor = FLinearColor::White;

		bool bShowStatus = true;

		if (InPlayerState->IsDead())
		{
			PlayerNameFontSize = 32;

			StatusFontSize = 16;
			StatusMessage = FText::FromString(TEXT("DEAD"));
			StatusColor = FLinearColor::Red;
		}
		else if (InPlayerState->HasFinishedGame())
		{
			PlayerNameFontSize = 32;

			StatusFontSize = 16;
			StatusMessage = FText::FromString(TEXT("ESCAPED"));
			StatusColor = FLinearColor::Green;
		}
		else if (InPlayerState->IsHost())
		{
			PlayerNameFontSize = 16;

			StatusFontSize = 8;
			StatusMessage = FText::FromString(TEXT("HOST"));
			StatusColor = FLinearColor::Green;
		}
		else
		{
			PlayerNameFontSize = 16;

			StatusFontSize = 8;
			bShowStatus = false;
		}

		FSlateFontInfo PlayerNameFontInfo = PlayerNameText->GetFont();
		PlayerNameFontInfo.Size = PlayerNameFontSize;
		PlayerNameText->SetFont(PlayerNameFontInfo);

		FSlateFontInfo StatusFontInfo = StatusText->GetFont();
		StatusFontInfo.Size = StatusFontSize;
		StatusText->SetFont(StatusFontInfo);
		StatusText->SetText(StatusMessage);
		StatusText->SetColorAndOpacity(StatusColor);
		StatusText->SetVisibility(bShowStatus ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
