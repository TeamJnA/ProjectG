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


void UPGFinalScoreBoardWidget::BindPlayerEntry(APlayerController* _PC)
{
	if (!_PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is valid. Binding delegate."));

	PCRef = _PC;

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->OnPlayerListUpdated.AddDynamic(this, &UPGFinalScoreBoardWidget::UpdatePlayerEntry);
	}

	UpdatePlayerEntry();
}

void UPGFinalScoreBoardWidget::UpdatePlayerEntry()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("UPGFinalScoreBoardWidget::UpdatePlayerEntry: Get world is null"));
		return;
	}

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	ensureMsgf(GS, TEXT("FinalScoreBoardWidget::UpdatePlayerEntry: GS is null"));

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	ensureMsgf(GI, TEXT("FinalScoreBoardWidget::UpdatePlayerEntry: GI is null"));

	PlayerContainer->ClearChildren();

	for (const FPlayerInfo& PlayerInfo : GS->PlayerList)
	{
		UPGPlayerEntryWidget* NewSlot = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
		if (NewSlot)
		{
			UTexture2D* AvatarTexture = nullptr;
			if (PlayerInfo.PlayerNetId.IsValid())
			{
				AvatarTexture = GI->GetSteamAvatarAsTexture(*PlayerInfo.PlayerNetId.GetUniqueNetId());
			}

			NewSlot->SetupEntry(FText::FromString(PlayerInfo.PlayerName), AvatarTexture);
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

void UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToMainMenuButtonClicked: Clicked"));

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->LeaveSessionAndReturnToLobby();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToMainMenuButtonClicked: Failed to get GI"));
	}
}

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
