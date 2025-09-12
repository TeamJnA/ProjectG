// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGPlayerEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Type/CharacterTypes.h"

/*
* 받은 데이터를 바탕으로 플레이어 이미지, 이름, 상태 디스플레이
* 탈출/사망 상태 -> ScoreBoardWidget, FinalScoreBoardWidget
* 호스트 여부 -> LobbyWidget
*/
void UPGPlayerEntryWidget::SetupEntry(const FPlayerInfo& InPlayerInfo, UTexture2D* InAvatarTexture)
{
	if (PlayerNameText) 
	{ 
		PlayerNameText->SetText(FText::FromString(InPlayerInfo.PlayerName));
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

		if (InPlayerInfo.bIsDead)
		{
			PlayerNameFontSize = 32;

			StatusFontSize = 16;
			StatusMessage = FText::FromString(TEXT("DEAD"));
			StatusColor = FLinearColor::Red;
		}
		else if (InPlayerInfo.bHasFinishedGame)
		{
			PlayerNameFontSize = 32;

			StatusFontSize = 16;
			StatusMessage = FText::FromString(TEXT("ESCAPED"));
			StatusColor = FLinearColor::Green;
		}
		else if (InPlayerInfo.bIsHost)
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
