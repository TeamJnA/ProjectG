// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGSpectatorWidget.h"

#include "Components/VerticalBox.h"
#include "UI/PGPlayerEntryWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Character/PGSpectatorPawn.h"

void UPGSpectatorWidget::Init()
{
	GIRef = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->OnPlayerArrayChanged.AddDynamic(this, &UPGSpectatorWidget::UpdatePlayerList);
	}

	APGSpectatorPawn* Spectator = GetOwningPlayerPawn<APGSpectatorPawn>();
	if (!Spectator)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpectatorWidget::Init: No valid spectator pawn"));
		return;
	}
	Spectator->OnSpectateTargetChanged.AddUniqueDynamic(this, &UPGSpectatorWidget::HandleSpectateTargetChanged);

	UpdatePlayerList();
}

void UPGSpectatorWidget::UpdatePlayerList()
{
	const APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GS || !PlayerListContainer || !PlayerEntryWidgetClass)
	{
		return;
	}

	PlayerEntries.Empty();
	PlayerListContainer->ClearChildren();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS == GetOwningPlayerState())
		{
			continue;
		}

		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			if (!PGPS->OnPlayerStateUpdated.IsAlreadyBound(this, &UPGSpectatorWidget::UpdatePlayerList))
			{
				PGPS->OnPlayerStateUpdated.AddDynamic(this, &UPGSpectatorWidget::UpdatePlayerList);
			}

			UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewPlayerEntry)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PGPS->GetUniqueId().IsValid())
				{
					AvatarTexture = GIRef->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
				}
				NewPlayerEntry->SetupEntry(PGPS, AvatarTexture, EPlayerEntryContext::Spectator);
				PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
				PlayerEntries.Add(NewPlayerEntry);
			}
		}
	}

	const APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		if (const APGSpectatorPawn* Spectator = PC->GetPawn<APGSpectatorPawn>())
		{
			HandleSpectateTargetChanged(Spectator->GetSpectateTargetPlayerState());
		}
	}
}

void UPGSpectatorWidget::HandleSpectateTargetChanged(const APlayerState* NewPlayerState)
{
	for (UPGPlayerEntryWidget* Entry : PlayerEntries)
	{
		if (Entry)
		{
			if (Entry->GetPlayerState() == NewPlayerState)
			{
				Entry->HighlightEntry();
			}
			else
			{
				Entry->UnhighlightEntry();
			}
		}
	}
}
