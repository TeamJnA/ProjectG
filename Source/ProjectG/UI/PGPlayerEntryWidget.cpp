// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGPlayerEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Player/PGPlayerState.h"

/*
* ������ ������ SetupEntry
* host ���� ����
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
* ���ھ��� �����ε� �Լ� ����
* host ���� ������ (false�� ȣ��)
*/
void UPGPlayerEntryWidget::SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture)
{
	// ������ ������ �Լ��� ȣ���ϵ�, bIsHostPlayer�� �⺻���� false�� �����մϴ�.
	SetupEntry(InPlayerName, InAvatarTexture, false);
}