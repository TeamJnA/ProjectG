// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGPlayerEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Player/PGPlayerState.h"

/*
* 완전한 버전의 SetupEntry
* host 여부 포함
*/
void UPGPlayerEntryWidget::SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture, bool bIsHostPlayer)
{
	if (PlayerNameText) { PlayerNameText->SetText(InPlayerName); }

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

	if (HostStatusText)
	{
		HostStatusText->SetVisibility(bIsHostPlayer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

/*
* 스코어보드용 오버로드 함수 구현
* host 여부 미포함 (false로 호출)
*/
void UPGPlayerEntryWidget::SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture)
{
	// 완전한 버전의 함수를 호출하되, bIsHostPlayer는 기본값인 false로 전달합니다.
	SetupEntry(InPlayerName, InAvatarTexture, false);
}