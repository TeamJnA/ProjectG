// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Player/PGLobbyPlayerController.h"

void UPGLobbyWidget::Init(APGLobbyPlayerController* PC)
{
	LocalPC = PC;

	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UPGLobbyWidget::OnReadyClicked);
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::Init: Bind button delegate"));
}

void UPGLobbyWidget::OnReadyClicked()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyWidget::OnReadyClicked: Ready button clicked"));

	if (LocalPC)
	{
		LocalPC->SetReady();
	}

	ReadyButton->SetIsEnabled(false);
	ReadyText->SetText(FText::FromString(TEXT("Waiting...")));
}

void UPGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
