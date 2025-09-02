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

void UPGScoreBoardWidget::BindPlayerEntry(APlayerController* _PC)
{
	if (!_PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is valid. Binding delegate."));

	PCRef = _PC;

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->OnPlayerListUpdated.AddDynamic(this, &UPGScoreBoardWidget::UpdatePlayerEntry);
	}

	UpdatePlayerEntry();
}

void UPGScoreBoardWidget::UpdatePlayerEntry()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreBoardWidget::UpdatePlayerEntry: Get world is null"));
		return;
	}

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	ensureMsgf(GS, TEXT("ScoreBoardWidget::UpdatePlayerEntry: GS is not valid"));

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	ensureMsgf(GI, TEXT("ScoreBoardWidget::UpdatePlayerEntry: GI is not valid"));

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

void UPGScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SpectateButton)
	{
		SpectateButton->OnClicked.AddDynamic(this, &UPGScoreBoardWidget::OnSpectateButtonClicked);
	}
}

void UPGScoreBoardWidget::OnSpectateButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: Spectate button clicked"), *PCRef->GetName());

	if (!PCRef)
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: PCRef is null"));
		return;
	}

	APGPlayerController* PGPC = Cast<APGPlayerController>(PCRef);
	if (!PGPC)
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: PC is not PG class"));
		return;
	}

	if (PGPC->IsLocalController())
	{
		PGPC->StartSpectate();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: PC is not Local"));
	}
}
