// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/Spectator/PGSpectatorWidget.h"
#include "UI/PlayerEntry/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Character/PGSpectatorPawn.h"

void UPGSpectatorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GIRef = GI;
	}
}

void UPGSpectatorWidget::NativeDestruct()
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

	if (APGSpectatorPawn* Spectator = SpectatorRef.Get())
	{
		Spectator->OnSpectateTargetChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPGSpectatorWidget::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("SpectatorWidget::Init"));

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GSRef = GS;
		GS->OnPlayerArrayChanged.AddUniqueDynamic(this, &UPGSpectatorWidget::UpdatePlayerList);

		for (APlayerState* PS : GS->PlayerArray)
		{
			if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
			{
				PGPS->OnPlayerStateUpdated.AddUniqueDynamic(this, &UPGSpectatorWidget::UpdatePlayerList);
			}
		}
	}

	if (APGSpectatorPawn* OldSpectator = SpectatorRef.Get())
	{
		OldSpectator->OnSpectateTargetChanged.RemoveAll(this);
	}

	if (APGSpectatorPawn* Spectator = GetOwningPlayerPawn<APGSpectatorPawn>())
	{
		SpectatorRef = Spectator;
		Spectator->OnSpectateTargetChanged.AddUniqueDynamic(this, &UPGSpectatorWidget::HandleSpectateTargetChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpectatorWidget::Init: No valid spectator pawn"));
	}

	UpdatePlayerList();
}

void UPGSpectatorWidget::UpdatePlayerList()
{
	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	APGGameState* GS = GSRef.Get();
	if (!GI || !GS || !PlayerListContainer || !PlayerEntryWidgetClass)
	{
		return;
	}

	PlayerEntries.Empty();
	PlayerListContainer->ClearChildren();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			UPGPlayerEntryWidget* NewPlayerEntry = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewPlayerEntry)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PGPS->GetUniqueId().IsValid())
				{
					AvatarTexture = GI->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
				}

				NewPlayerEntry->SetupEntry(PGPS, AvatarTexture, EPlayerEntryContext::Spectator);
				PlayerListContainer->AddChildToVerticalBox(NewPlayerEntry);
				PlayerEntries.Add(NewPlayerEntry);
			}
		}
	}

	APGSpectatorPawn* Spectator = SpectatorRef.Get();
	if (!Spectator)
	{
		return;
	}
	HandleSpectateTargetChanged(Spectator->GetSpectateTargetPlayerState());
}

void UPGSpectatorWidget::HandleSpectateTargetChanged(const APlayerState* NewPlayerState)
{
	for (UPGPlayerEntryWidget* Entry : PlayerEntries)
	{
		if (!Entry)
		{
			continue;
		}

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
