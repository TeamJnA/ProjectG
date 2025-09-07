// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGHUD.h"
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

void APGHUD::InitScoreBoardWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !ScoreBoardWidgetClass || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()) || (ScoreBoardWidget && ScoreBoardWidget->IsInViewport()))
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
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !FinalScoreBoardWidgetClass)
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
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PauseMenuWidgetClass || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()) || (PauseMenuWidget && PauseMenuWidget->IsInViewport()))
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

