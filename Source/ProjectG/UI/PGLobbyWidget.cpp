// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLobbyWidget.h"

#include "Kismet/GameplayStatics.h"

#include "Components/VerticalBox.h"
#include "UI/PGPlayerEntryWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"

/*
* ���� �÷��̾�� ������ LobbyWidget�� ������ ���� �÷��̾� ��� ������Ʈ
*/
void UPGLobbyWidget::Init()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::Init: lobby widget init"));

	UpdatePlayerList();
}

void UPGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GIRef = GetGameInstance<UPGAdvancedFriendsGameInstance>();

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->OnPlayerListUpdated.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);
	}
}

/*
* ���ǿ� �÷��̾� �߰�/���� �� ��������Ʈ�� ���� ȣ��
* ���� ������ �÷��̾� ��� ������Ʈ
* Host�� ���� ���� ������ ����
* �÷��̾��� Steam ������ �̹���, �̸�, Host ���� ���÷���
*/
void UPGLobbyWidget::UpdatePlayerList()
{
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GS || !PlayerListContainer || !PlayerEntryWidgetClass)
	{
		return;
	}

	PlayerListContainer->ClearChildren();

	// GameState .h
	// const TArray<FPlayerInfo>& GetPlayerList() const { return PlayerList; }
	const TArray<FPlayerInfo>& PlayerList = GS->GetPlayerList();
	for (const FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (PlayerInfo.bIsHost)
		{
			UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewPlayerEntry)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PlayerInfo.PlayerNetId.IsValid())
				{
					AvatarTexture = GIRef->GetSteamAvatarAsTexture(*PlayerInfo.PlayerNetId.GetUniqueNetId());
				}
				NewPlayerEntry->SetupEntry(PlayerInfo, AvatarTexture);
				PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
			}
		}
	}

	for (const FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (!PlayerInfo.bIsHost)
		{
			UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewPlayerEntry)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PlayerInfo.PlayerNetId.IsValid())
				{
					AvatarTexture = GIRef->GetSteamAvatarAsTexture(*PlayerInfo.PlayerNetId.GetUniqueNetId());
				}
				NewPlayerEntry->SetupEntry(PlayerInfo, AvatarTexture);
				PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
			}
		}
	}
}
