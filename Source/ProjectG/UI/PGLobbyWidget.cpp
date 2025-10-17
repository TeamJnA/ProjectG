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
		GS->OnPlayerArrayChanged.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);
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

	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::UpdatePlayerList: Update"));

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			if (!PGPS->OnPlayerStateUpdated.IsAlreadyBound(this, &UPGLobbyWidget::UpdatePlayerList))
			{
				PGPS->OnPlayerStateUpdated.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);
			}
		}
	}

	PlayerListContainer->ClearChildren();

	// GameState .h
	// const TArray<FPlayerInfo>& GetPlayerList() const { return PlayerList; }
	for (APlayerState* PS : GS->PlayerArray)
	{
		UE_LOG(LogTemp, Log, TEXT("LobbyWidget::UpdatePlayerList: [%s]"), *PS->GetPlayerName());

		if (const APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			if (PGPS->IsHost())
			{
				UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
				if (NewPlayerEntry)
				{
					UTexture2D* AvatarTexture = nullptr;
					if (PGPS->GetUniqueId().IsValid())
					{
						AvatarTexture = GIRef->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
					}
					NewPlayerEntry->SetupEntry(PGPS, AvatarTexture, EPlayerEntryContext::Lobby);
					PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
				}
			}
		}
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (const APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			if (!PGPS->IsHost())
			{
				UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
				if (NewPlayerEntry)
				{
					UTexture2D* AvatarTexture = nullptr;
					if (PGPS->GetUniqueId().IsValid())
					{
						AvatarTexture = GIRef->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
					}
					NewPlayerEntry->SetupEntry(PGPS, AvatarTexture, EPlayerEntryContext::Lobby);
					PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
				}
			}
		}
	}
}
