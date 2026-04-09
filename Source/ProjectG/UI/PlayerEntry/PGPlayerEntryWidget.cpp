// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/PGPlayerEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

#include "Player/PGPlayerState.h"
#include "Type/PGPhotoTypes.h"


/*
* 받은 데이터를 바탕으로 플레이어 이미지, 이름, 상태 디스플레이
* 탈출/사망 상태 -> ScoreBoardWidget, FinalScoreBoardWidget
* 호스트 여부 -> LobbyWidget
*/
void UPGPlayerEntryWidget::SetupEntry(const APGPlayerState* InPlayerState, UTexture2D* InAvatarTexture, EPlayerEntryContext Context)
{
	UE_LOG(LogTemp, Log, TEXT("PlayerEntryWidget::SetupEntry: [%s]"), *InPlayerState->GetPlayerName());
	
	if (!InPlayerState || !PlayerNameText || !PlayerAvatar || !StatusText || !ScoreText)
	{
		return;
	}
	PlayerStateRef = const_cast<APGPlayerState*>(InPlayerState);

	PlayerNameText->SetText(FText::FromString(InPlayerState->GetPlayerName()));

	const bool bHasAvatar = (InAvatarTexture != nullptr);
	if (bHasAvatar)
	{
		PlayerAvatar->SetBrushFromTexture(InAvatarTexture);
	}
	PlayerAvatar->SetVisibility(bHasAvatar ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	int32 PlayerNameFontSize;
	int32 StatusFontSize;
	FText StatusMessage;
	FLinearColor StatusColor = FLinearColor::White;
	bool bShowStatus = true;
	bool bShowScore = false;

	if (Context == EPlayerEntryContext::Lobby)
	{
		PlayerNameFontSize = 16;
		StatusFontSize = 8;
		if (InPlayerState->IsHost())
		{
			StatusMessage = FText::FromString(TEXT("HOST"));
			StatusColor = FLinearColor::Green;
		}
		else
		{
			bShowStatus = false;
		}
	}
	else if (Context == EPlayerEntryContext::Scoreboard)
	{
		PlayerNameFontSize = 28;
		StatusFontSize = 24;
		if (InPlayerState->IsDead())
		{
			StatusMessage = FText::FromString(TEXT("DEAD"));
			StatusColor = FLinearColor::Red;
		}
		else if (InPlayerState->HasFinishedGame())
		{
			StatusMessage = FText::FromString(TEXT("ESCAPED"));
			StatusColor = FLinearColor::Green;
		}
	}
	else if (Context == EPlayerEntryContext::FinalScoreboard)
	{
		PlayerNameFontSize = 28;
		StatusFontSize = 24;
		bShowScore = true;
		if (InPlayerState->IsDead())
		{
			StatusMessage = FText::FromString(TEXT("DEAD"));
			StatusColor = FLinearColor::Red;
		}
		else if (InPlayerState->HasFinishedGame())
		{
			StatusMessage = FText::FromString(TEXT("ESCAPED"));
			StatusColor = FLinearColor::Green;
		}
	}
	else
	{
		PlayerNameFontSize = 16;
		StatusFontSize = 8;
		if (InPlayerState->IsDead())
		{
			StatusMessage = FText::FromString(TEXT("DEAD"));
			StatusColor = FLinearColor::Red;
		}
		else if (InPlayerState->HasFinishedGame())
		{
			StatusMessage = FText::FromString(TEXT("ESCAPED"));
			StatusColor = FLinearColor::Green;
		}
	}

	FSlateFontInfo PlayerNameFontInfo = PlayerNameText->GetFont();
	PlayerNameFontInfo.Size = PlayerNameFontSize;
	PlayerNameText->SetFont(PlayerNameFontInfo);

	FSlateFontInfo StatusFontInfo = StatusText->GetFont();
	StatusFontInfo.Size = StatusFontSize;
	StatusText->SetFont(StatusFontInfo);
	StatusText->SetText(StatusMessage);
	StatusText->SetColorAndOpacity(StatusColor);
	StatusText->SetVisibility(bShowStatus ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	int32 Score = InPlayerState->GetPhotoScore();
	ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *PhotoGrade::GetGrade(Score))));
	ScoreText->SetVisibility(bShowScore ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	UnhighlightEntry();
}

void UPGPlayerEntryWidget::HighlightEntry()
{
	if (HighlightBorder)
	{
		HighlightBorder->SetBrushColor(HighlightColor);
	}
}

void UPGPlayerEntryWidget::UnhighlightEntry()
{
	if (HighlightBorder)
	{
		HighlightBorder->SetBrushColor(DefaultColor);
	}
}

void UPGPlayerEntryWidget::HideScoreText()
{
	if (ScoreText)
	{
		ScoreText->SetRenderOpacity(0.0f);
	}
}

void UPGPlayerEntryWidget::PlayGradeStampEffect()
{
	if (GradeStampAnim)
	{
		PlayAnimation(GradeStampAnim);
	}
}
