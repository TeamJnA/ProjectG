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
		GS->OnLobbyPlayerListUpdated.AddDynamic(this, &UPGLobbyWidget::UpdatePlayerList);
	}
}

void UPGLobbyWidget::UpdatePlayerList()
{
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GS || !PlayerListContainer || !PlayerEntryWidgetClass)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::UpdatePlayerList: Refreshing UI with %d player(s)."), GS->LobbyPlayerList.Num());

	PlayerListContainer->ClearChildren();

	// GameState에 복제된 최신 목록을 정렬합니다.
	TArray<FPlayerLobbyInfo> SortedInfos = GS->LobbyPlayerList;
	SortedInfos.Sort([](const FPlayerLobbyInfo& A, const FPlayerLobbyInfo& B) 
	{
		return A.bIsHost > B.bIsHost;
	});

	for (const FPlayerLobbyInfo& Info : SortedInfos)
	{
		UTexture2D* AvatarTexture = nullptr;
		if (Info.AvatarRawData.Num() > 0)
		{
			// 받은 데이터로 텍스처를 생성합니다.
			AvatarTexture = UTexture2D::CreateTransient(Info.AvatarWidth, Info.AvatarHeight, PF_R8G8B8A8);
			if (AvatarTexture)
			{
				void* TextureData = AvatarTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, Info.AvatarRawData.GetData(), Info.AvatarRawData.Num());
				AvatarTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
				AvatarTexture->UpdateResource();
			}
		}

		UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
		if (NewPlayerEntry)
		{
			NewPlayerEntry->SetupEntry(FText::FromString(Info.PlayerName), AvatarTexture, Info.bIsHost);
			PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
		}
	}
}
