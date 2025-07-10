// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGScoreBoardWidget.h"
#include "UI/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Character/PGPlayerCharacter.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"

void UPGScoreBoardWidget::BindPlayerEntry(APGPlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is valid. Binding delegate."));

	PlayerRef = PlayerCharacter;

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
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreBoardWidget::UpdatePlayerEntry: Get GameState is null"));
		return;
	}

	PlayerContainer->ClearChildren();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			UPGPlayerEntryWidget* NewSlot = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewSlot)
			{
				PlayerContainer->AddChild(NewSlot);
				NewSlot->SetPlayerText(FText::FromString(PGPS->GetPlayerName()));
				UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::UpdatePlayerEntry: Add PlayerEntry | Name: %s"), *PGPS->GetPlayerName());
			}
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
	UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: Spectate button clicked"), *PlayerRef->GetName());

	if (!PlayerRef)
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: PlayerRef is null"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PlayerRef->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: PC is null"));
		return;
	}

	APGPlayerController* PGPC = Cast<APGPlayerController>(PC);
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
