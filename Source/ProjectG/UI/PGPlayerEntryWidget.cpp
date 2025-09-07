// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGPlayerEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Type/CharacterTypes.h"

void UPGPlayerEntryWidget::SetupEntry(const FPlayerInfo& InPlayerInfo, UTexture2D* InAvatarTexture)
{
	if (PlayerNameText) 
	{ 
		PlayerNameText->SetText(FText::FromString(InPlayerInfo.PlayerName));
	}

	if (PlayerAvatar)
	{
		if (InAvatarTexture)
		{
			PlayerAvatar->SetBrushFromTexture(InAvatarTexture);
			PlayerAvatar->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			PlayerAvatar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (StatusText)
	{
		FText StatusMessage;
		FLinearColor StatusColor = FLinearColor::White;
		FSlateFontInfo StatusFontInfo = StatusText->GetFont();
		FSlateFontInfo PlayerNameFontInfo = PlayerNameText->GetFont();

		bool bShowStatus = true;

		// scoreboard or finalscoreboard
		if (InPlayerInfo.bIsDead)
		{
			StatusMessage = FText::FromString(TEXT("DEAD"));
			StatusColor = FLinearColor::Red;
			StatusFontInfo.Size = 16;
			PlayerNameFontInfo.Size = 32;
		}
		// scoreboard or finalscoreboard
		else if (InPlayerInfo.bHasFinishedGame)
		{
			StatusMessage = FText::FromString(TEXT("ESCAPED"));
			StatusColor = FLinearColor::Green;
			StatusFontInfo.Size = 16;
			PlayerNameFontInfo.Size = 32;
		}
		// lobby widget host
		else if (InPlayerInfo.bIsHost)
		{
			StatusMessage = FText::FromString(TEXT("HOST"));
			StatusColor = FLinearColor::Green;
			StatusFontInfo.Size = 8;
			PlayerNameFontInfo.Size = 16;
		}
		// lobby widget client
		else
		{
			bShowStatus = false;
			StatusFontInfo.Size = 8;
			PlayerNameFontInfo.Size = 16;
		}

		StatusText->SetText(StatusMessage);
		StatusText->SetColorAndOpacity(StatusColor);
		StatusText->SetFont(StatusFontInfo);
		StatusText->SetVisibility(bShowStatus ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		PlayerNameText->SetFont(PlayerNameFontInfo);
	}
}
