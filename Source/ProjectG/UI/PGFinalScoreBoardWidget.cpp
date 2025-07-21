// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGFinalScoreBoardWidget.h"
#include "UI/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Character/PGPlayerCharacter.h"

#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"

#include "Kismet/GameplayStatics.h"

void UPGFinalScoreBoardWidget::BindPlayerEntry(APlayerController* _PC)
{
	if (!_PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is valid. Binding delegate."));

	PCRef = _PC;

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
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGFinalScoreBoardWidget::UpdatePlayerEntry: Get GameState is null"));
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
