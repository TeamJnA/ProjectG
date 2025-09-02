// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLobbyWidget.h"

#include "Kismet/GameplayStatics.h"

#include "Components/VerticalBox.h"
#include "UI/PGPlayerEntryWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGLobbyPlayerController.h"

void UPGLobbyWidget::Init(APGLobbyPlayerController* PC)
{
	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::Init: lobby widget init"));
	LocalPC = PC;

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

	// GameState�� ����� PlayerList�� Host�� ���� ���� ������ ����
	TArray<FPlayerInfo> SortedInfos = GS->PlayerList;
	SortedInfos.Sort([](const FPlayerInfo& A, const FPlayerInfo& B)
	{
		return A.bIsHost > B.bIsHost;
	});

	for (const FPlayerInfo& Info : SortedInfos)
	{
		UTexture2D* AvatarTexture = nullptr;
		if (Info.PlayerNetId.IsValid())
		{
			AvatarTexture = GIRef->GetSteamAvatarAsTexture(*Info.PlayerNetId.GetUniqueNetId());
		}

		UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
		if (NewPlayerEntry)
		{
			NewPlayerEntry->SetupEntry(FText::FromString(Info.PlayerName), AvatarTexture, Info.bIsHost);
			PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
		}
	}
}
