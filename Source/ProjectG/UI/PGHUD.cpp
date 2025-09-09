// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGHUD.h"
#include "UI/PGMainMenuWidget.h"
#include "UI/PGLobbyWidget.h"
#include "UI/PGAttributesWidget.h"
#include "UI/PGInventoryWidget.h"
#include "UI/PGMessageManagerWidget.h"
#include "UI/PGScoreBoardWidget.h"
#include "UI/PGFinalScoreBoardWidget.h"
#include "UI/PGPauseMenuWidget.h"
#include "UI/PGCrosshairWidget.h"
#include "UI/PGInteractionProgressWidget.h"

#include "Character/Component/PGInventoryComponent.h"

APGHUD::APGHUD()
{
}

void APGHUD::Init()
{
	AttributeWidget = CreateWidget<UPGAttributesWidget>(GetOwningPlayerController(), AttributeWidgetClass);
	AttributeWidget->BindToAttributes();
	AttributeWidget->AddToViewport();

	InventoryWidget = CreateWidget<UPGInventoryWidget>(GetOwningPlayerController(), InventoryWidgetClass);
	if (InventoryWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: InventoryWidget created successfully."));
		// InventoryWidget->BindMessageEntry();
		InventoryWidget->AddToViewport();
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: InventoryWidget added to viewport."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("APGHUD::Init: Failed to create InventoryWidget! Check InventoryWidgetClass in HUD Blueprint."));
	}

	MessageManagerWidget = CreateWidget<UPGMessageManagerWidget>(GetOwningPlayerController(), MessageManagerWidgetClass);
	if (MessageManagerWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: MessageManagerWidget created successfully."));
		// MessageManagerWidget->BindMessageEntry();
		MessageManagerWidget->AddToViewport();
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: MessageManagerWidget added to viewport."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("APGHUD::Init: Failed to create MessageManagerWidget! Check MessageManagerWidgetClass in HUD Blueprint."));
	}

	CrosshairWidget = CreateWidget<UPGCrosshairWidget>(GetOwningPlayerController(), CrosshairWidgetClass);
	CrosshairWidget->AddToViewport();

	InteractionProgressWidget = CreateWidget<UPGInteractionProgressWidget>(GetOwningPlayerController(), InteractionProgressWidgetClass);
}

void APGHUD::InitMainMenuWidget()
{
	if (!MainMenuWidgetClass || (MainMenuWidget && MainMenuWidget->IsInViewport()))
	{
		return;
	}
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UPGMainMenuWidget>(PC, MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void APGHUD::InitLobbyWidget()
{
	if (!LobbyWidgetClass || (LobbyWidget && LobbyWidget->IsInViewport()))
	{
		return;
	}
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (MainMenuWidget && MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	LobbyWidget = CreateWidget<UPGLobbyWidget>(PC, LobbyWidgetClass);
	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport();
		LobbyWidget->Init();

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void APGHUD::InitScoreBoardWidget()
{
	if (!ScoreBoardWidgetClass || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()) || (ScoreBoardWidget && ScoreBoardWidget->IsInViewport()))
	{
		return;
	}
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeUIOnly());

	ScoreBoardWidget = CreateWidget<UPGScoreBoardWidget>(PC, ScoreBoardWidgetClass);
	if (ScoreBoardWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("APGHUD::InitScoreBoardWidget: ScoreBoardWidget created successfully."));
		ScoreBoardWidget->AddToViewport();
		ScoreBoardWidget->BindPlayerEntry(PC);
		UE_LOG(LogTemp, Log, TEXT("APGHUD::InitScoreBoardWidget: ScoreBoardWidget added to viewport."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("APGHUD::InitScoreBoardWidget: Failed to create ScoreBoardWidget! Check ScoreBoardWidgetClass in HUD Blueprint."));
	}
}

void APGHUD::InitFinalScoreBoardWidget()
{
	if (!FinalScoreBoardWidgetClass || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()))
	{
		return;
	}
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (UGameViewportClient* ViewPort = GetWorld()->GetGameViewport())
	{
		ViewPort->RemoveAllViewportWidgets();
	}

	FinalScoreBoardWidget = CreateWidget<UPGFinalScoreBoardWidget>(PC, FinalScoreBoardWidgetClass);
	if (FinalScoreBoardWidget)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());

		FinalScoreBoardWidget->AddToViewport();
		FinalScoreBoardWidget->BindPlayerEntry(PC);
	}
}

void APGHUD::InitPauseMenuWidget()
{
	if (!PauseMenuWidgetClass || (MainMenuWidget && MainMenuWidget->IsInViewport()) || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()) || (PauseMenuWidget && PauseMenuWidget->IsInViewport()))
	{
		return;
	}
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	PauseMenuWidget = CreateWidget<UPGPauseMenuWidget>(PC, PauseMenuWidgetClass);
	if (PauseMenuWidget)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());

		PauseMenuWidget->AddToViewport();
		PauseMenuWidget->Init(PC);
	}
}

void APGHUD::UpdateInteractionProgress(float Progress)
{
	if (!InteractionProgressWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD::UpdateInteractionProgress: No valid InteractionProgressWidget"));
		return;
	}

	if (Progress > 0.0f)
	{
		if (!InteractionProgressWidget->IsInViewport())
		{
			InteractionProgressWidget->AddToViewport();
		}
		InteractionProgressWidget->SetProgress(Progress);
	}
	else
	{
		if (InteractionProgressWidget->IsInViewport())
		{
			InteractionProgressWidget->RemoveFromParent();
		}
	}
}

void APGHUD::DisplayInteractionFailedMessage(const FText& Message, float Duration)
{
	if (MessageManagerWidget)
	{
		MessageManagerWidget->ShowFailureMessage(Message, Duration);
	}
}

