// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/Lobby/PGLobbyWidget.h"
#include "UI/PlayerEntry/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"

#include "Kismet/GameplayStatics.h"

void UPGLobbyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GIRef = GI;
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GSRef = GS;
		GS->OnPlayerArrayChanged.RemoveAll(this);
		GS->OnPlayerArrayChanged.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);
	}
}

void UPGLobbyWidget::NativeDestruct()
{
	if (APGGameState* GS = GSRef.Get())
	{
		GS->OnPlayerArrayChanged.RemoveAll(this);

		for (APlayerState* PS : GS->PlayerArray)
		{
			if (!IsValid(PS))
			{
				continue;
			}

			if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
			{
				PGPS->OnPlayerStateUpdated.RemoveAll(this);
			}
		}
	}

	Super::NativeDestruct();
}

/*
* 로컬 플레이어에게 완전히 LobbyWidget이 생성된 이후 플레이어 목록 업데이트
*/
void UPGLobbyWidget::Init()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::Init: lobby widget init"));

	UpdatePlayerList();
}

/*
* 세션에 플레이어 추가/제거 시 델리게이트를 통해 호출
* 현재 세션의 플레이어 목록 업데이트
* Host가 가장 위로 오도록 정렬
* 플레이어의 Steam 프로필 이미지, 이름, Host 여부 디스플레이
*/
void UPGLobbyWidget::UpdatePlayerList()
{
	if (!PlayerListContainer || !PlayerEntryWidgetClass)
	{
		return;
	}
	
	APGGameState* GS = GSRef.Get();
	if (!GS)
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	if (!GI)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::UpdatePlayerList: Update"));

	PlayerListContainer->ClearChildren();

	APGPlayerState* HostPlayer = nullptr;
	TArray<APGPlayerState*> GuestPlayers;
	GuestPlayers.Reserve(GS->PlayerArray.Num());

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			PGPS->OnPlayerStateUpdated.RemoveAll(this);
			PGPS->OnPlayerStateUpdated.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);

			if (PGPS->IsHost())
			{
				HostPlayer = PGPS;
			}
			else
			{
				GuestPlayers.Add(PGPS);
			}
		}
	}

	auto CreateEntry = [&](APGPlayerState* PGPS)
	{
		if (!PGPS)
		{
			return;
		}

		UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
		if (NewPlayerEntry)
		{
			UTexture2D* AvatarTexture = nullptr;
			if (PGPS->GetUniqueId().IsValid())
			{
				AvatarTexture = GI->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
			}
			NewPlayerEntry->SetupEntry(PGPS, AvatarTexture, EPlayerEntryContext::Lobby);
			PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
		}
	};

	if (HostPlayer)
	{
		CreateEntry(HostPlayer);
	}

	for (APGPlayerState* Guest : GuestPlayers)
	{
		CreateEntry(Guest);
	}
}
