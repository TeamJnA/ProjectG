// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGScoreBoardWidget.h"

#include "UI/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Character/PGPlayerCharacter.h"

/*
* ���� �÷��̾ ScoreBoardWidget ���� ���� �÷��̾� ��� ������Ʈ
* GameState�� PlayerList ������Ʈ �� ���޵Ǵ� OnPlayerListUpdated ��������Ʈ�� ������Ʈ �Լ� ���ε�
*/
void UPGScoreBoardWidget::BindPlayerEntry(APlayerController* InPC)
{
	if (!InPC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is valid. Binding delegate."));

	PCRef = InPC;

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->OnPlayerListUpdated.AddDynamic(this, &UPGScoreBoardWidget::UpdatePlayerEntry);
	}

	UpdatePlayerEntry();
}

/*
* �÷��̾� ��� ������Ʈ ������
* ���� ���� ������ �÷��̾���� Steam ������ �̹���, �̸�, ���� ���� ���÷���
*/
void UPGScoreBoardWidget::UpdatePlayerEntry()
{
	if (!GetWorld())
	{
		return;
	}

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GS || !GI)
	{
		return;
	}

	PlayerContainer->ClearChildren();

	const TArray<FPlayerInfo>& PlayerList = GS->GetPlayerList();
	for (const FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (PlayerInfo.bHasFinishedGame)
		{
			UPGPlayerEntryWidget* NewSlot = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewSlot)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PlayerInfo.PlayerNetId.IsValid())
				{
					AvatarTexture = GI->GetSteamAvatarAsTexture(*PlayerInfo.PlayerNetId.GetUniqueNetId());
				}

				NewSlot->SetupEntry(PlayerInfo, AvatarTexture);
				PlayerContainer->AddChild(NewSlot);
				UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::UpdatePlayerEntry: Add PlayerEntry | Name: %s"), *PlayerInfo.PlayerName);
			}
		}
	}
}

void UPGScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SpectateButton)
	{
		SpectateButton->OnClicked.AddDynamic(this, &UPGScoreBoardWidget::OnSpectateButtonClicked);
	}
}

/*
* ���� ��ư Ŭ�� �� ���� ��� ����
*/
void UPGScoreBoardWidget::OnSpectateButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: Spectate button clicked"), *PCRef->GetName());
	
	APGPlayerController* PGPC = Cast<APGPlayerController>(PCRef);
	if (!PGPC)
	{
		return;
	}

	if (PGPC->IsLocalController())
	{
		PGPC->StartSpectate();
	}
}
