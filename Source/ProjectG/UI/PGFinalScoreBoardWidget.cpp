// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGFinalScoreBoardWidget.h"

#include "Kismet/GameplayStatics.h"

#include "UI/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Character/PGPlayerCharacter.h"

/*
* ���� �÷��̾ FinalScoreBoardWidget ���� ���� �÷��̾� ��� ������Ʈ
* GameState�� PlayerList ������Ʈ �� ���޵Ǵ� OnPlayerListUpdated ��������Ʈ�� ������Ʈ �Լ� ���ε�
*/
void UPGFinalScoreBoardWidget::BindPlayerEntry(APlayerController* InPC)
{
	if (!InPC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is valid. Binding delegate."));

	PCRef = InPC;

	if (UWorld* World = GetWorld())
	{
		if (APGGameState* GS = World->GetGameState<APGGameState>())
		{
			GSRef = GS;
			GS->OnPlayerListUpdated.AddDynamic(this, &UPGFinalScoreBoardWidget::UpdatePlayerEntry);
		}
	}

	UpdatePlayerEntry();
}

/*
* �÷��̾� ��� ������Ʈ ������
* ��� �÷��̾���� Steam ������ �̹���, �̸�, ���� ���� ���÷���
*/
void UPGFinalScoreBoardWidget::UpdatePlayerEntry()
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

void UPGFinalScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReturnToMainMenuButton)
	{
		ReturnToMainMenuButton->OnClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked);
	}

	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::OnReturnToLobbyButtonClicked);
	}
}

void UPGFinalScoreBoardWidget::NativeDestruct()
{
	if (GSRef.IsValid())
	{
		GSRef->OnPlayerListUpdated.RemoveDynamic(this, &UPGFinalScoreBoardWidget::UpdatePlayerEntry);
		UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget: OnPlayerListUpdated delegate unbound."));
	}

	Super::NativeDestruct();
}

/*
* ���� �޴� ��ư -> ���� ���� �� ���� �޴��� �̵�
*/
void UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToMainMenuButtonClicked: Clicked"));
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::OnReturnToMainMenuButtonClicked: No valid GI"));
	}
}

/*
* �κ�� ������ ��ư -> ������ ������ ä �κ�� �̵�
* �ٸ� �÷��̾���� ������ ��ٸ� �� 
* �κ�� ������ ��ư�� Ŭ���� �÷��̾�鿡 ���� ������ ������ ä �κ�� �̵�
*/
void UPGFinalScoreBoardWidget::OnReturnToLobbyButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: Clicked"));
	if (PCRef)
	{
		if (APGPlayerController* PC = Cast<APGPlayerController>(PCRef))
		{
			PC->NotifyReadyToReturnLobby();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: PC Ref is not PG class"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: No PC Ref"));
	}
}
