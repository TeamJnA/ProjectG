// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLobbyWidget.h"

#include "Kismet/GameplayStatics.h"

#include "Components/VerticalBox.h"
#include "UI/PGPlayerEntryWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"

/*
* 로컬 플레이어에게 완전히 LobbyWidget이 생성된 이후 플레이어 목록 업데이트
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
* 세션에 플레이어 추가/제거 시 델리게이트를 통해 호출
* 현재 세션의 플레이어 목록 업데이트
* Host가 가장 위로 오도록 정렬
* 플레이어의 Steam 프로필 이미지, 이름, Host 여부 디스플레이
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
