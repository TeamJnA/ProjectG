// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLobbyWidget.h"

#include "Kismet/GameplayStatics.h"

#include "Components/VerticalBox.h"
#include "UI/PGPlayerEntryWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"

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
		UE_LOG(LogTemp, Log, TEXT("LobbyWidget::NativeConstruct: lobby widget construct"));
		GS->OnPlayerListUpdated.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);
	}
}

void UPGLobbyWidget::UpdatePlayerList()
{
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GS || !PlayerListContainer || !PlayerEntryWidgetClass)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::UpdatePlayerList: Refreshing UI with %d player(s)."), GS->PlayerList.Num());

	PlayerListContainer->ClearChildren();

	// GameState에 저장된 PlayerList를 Host가 가장 위로 오도록 정렬
	TArray<FPlayerInfo> SortedInfos = GS->PlayerList;
	SortedInfos.Sort([](const FPlayerInfo& A, const FPlayerInfo& B)
	{
		return A.bIsHost > B.bIsHost;
	});

	for (const FPlayerInfo& PlayerInfo : SortedInfos)
	{
		UTexture2D* AvatarTexture = nullptr;
		if (PlayerInfo.PlayerNetId.IsValid())
		{
			AvatarTexture = GIRef->GetSteamAvatarAsTexture(*PlayerInfo.PlayerNetId.GetUniqueNetId());
		}

		UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
		if (NewPlayerEntry)
		{
			NewPlayerEntry->SetupEntry(PlayerInfo, AvatarTexture);
			PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
		}
	}
}
