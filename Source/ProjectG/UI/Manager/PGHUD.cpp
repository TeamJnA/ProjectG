// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Manager/PGHUD.h"

#include "UI/Menu/PGMainMenuWidget.h"
#include "UI/Menu/PGPauseMenuWidget.h"

#include "UI/PlayerEntry/Lobby/PGLobbyWidget.h"
#include "UI/PlayerEntry/Spectator/PGSpectatorWidget.h"
#include "UI/PlayerEntry/ScoreBoard/PGScoreBoardWidget.h"
#include "UI/PlayerEntry/ScoreBoard/PGFinalScoreBoardWidget.h"

#include "UI/HUD/PGAttributesWidget.h"
#include "UI/HUD/PGInventoryWidget.h"
#include "UI/HUD/PGMessageManagerWidget.h"
#include "UI/HUD/PGCrosshairWidget.h"
#include "UI/HUD/PGInteractionProgressWidget.h"
#include "UI/Screen/PGJumpscareWidget.h"

#include "Character/Component/PGInventoryComponent.h"

APGHUD::APGHUD()
{
}

/*
* 인게임 HUD 구성
*/
void APGHUD::Init()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!AttributeWidget)
	{
		AttributeWidget = CreateWidget<UPGAttributesWidget>(PC, AttributeWidgetClass);
	}

	if (AttributeWidget && !AttributeWidget->IsInViewport())
	{
		AttributeWidget->BindToAttributes();
		AttributeWidget->AddToViewport();
	}

	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UPGInventoryWidget>(PC, InventoryWidgetClass);
	}

	if (InventoryWidget && !InventoryWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: InventoryWidget created successfully."));
		InventoryWidget->AddToViewport();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("APGHUD::Init: Failed to create InventoryWidget! Check InventoryWidgetClass in HUD Blueprint."));
	}

	if (!MessageManagerWidget)
	{
		MessageManagerWidget = CreateWidget<UPGMessageManagerWidget>(PC, MessageManagerWidgetClass);
	}

	if (MessageManagerWidget && !MessageManagerWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: MessageManagerWidget created successfully."));
		MessageManagerWidget->AddToViewport();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("APGHUD::Init: Failed to create MessageManagerWidget! Check MessageManagerWidgetClass in HUD Blueprint."));
	}

	if (!CrosshairWidget)
	{
		CrosshairWidget = CreateWidget<UPGCrosshairWidget>(PC, CrosshairWidgetClass);
	}

	if (CrosshairWidget && !CrosshairWidget->IsInViewport())
	{
		CrosshairWidget->AddToViewport();
	}

	if (!InteractionProgressWidget)
	{
		InteractionProgressWidget = CreateWidget<UPGInteractionProgressWidget>(PC, InteractionProgressWidgetClass);
	}
}

/*
* MainMenuWidget 생성
*/
void APGHUD::InitMainMenuWidget()
{
	if (!MainMenuWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UPGMainMenuWidget>(PC, MainMenuWidgetClass);
	}

	if (MainMenuWidget && !MainMenuWidget->IsInViewport())
	{
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;

		MainMenuWidget->AddToViewport();
	}
}

/*
* LobbyWidget 생성
*/
void APGHUD::InitLobbyWidget()
{
	if (!LobbyWidgetClass)
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
	}

	if (!LobbyWidget)
	{
		LobbyWidget = CreateWidget<UPGLobbyWidget>(PC, LobbyWidgetClass);
	}

	if (LobbyWidget && !LobbyWidget->IsInViewport())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;

		LobbyWidget->Init();
		LobbyWidget->AddToViewport();
	}
}

void APGHUD::InitSpectatorWidget()
{
	if (!SpectatorWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!SpectatorWidget)
	{
		SpectatorWidget = CreateWidget<UPGSpectatorWidget>(PC, SpectatorWidgetClass);
	}

	if (SpectatorWidget && !SpectatorWidget->IsInViewport())
	{
		SpectatorWidget->Init();
		SpectatorWidget->AddToViewport();
	}
}

/*
* ScoreBoardWidget 생성
*/
void APGHUD::InitScoreBoardWidget()
{
	if (!ScoreBoardWidgetClass || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()))
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!ScoreBoardWidget)
	{
		ScoreBoardWidget = CreateWidget<UPGScoreBoardWidget>(PC, ScoreBoardWidgetClass);
	}

	if (ScoreBoardWidget && !ScoreBoardWidget->IsInViewport())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
		ScoreBoardWidget->BindPlayerEntry();
		ScoreBoardWidget->AddToViewport();
	}
}

/*
* FinalScoreBoardWidget 생성
*/
void APGHUD::InitFinalScoreBoardWidget()
{
	if (!FinalScoreBoardWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!FinalScoreBoardWidget)
	{
		FinalScoreBoardWidget = CreateWidget<UPGFinalScoreBoardWidget>(PC, FinalScoreBoardWidgetClass);
	}

	if (FinalScoreBoardWidget && !FinalScoreBoardWidget->IsInViewport())
	{
		ClearViewport();

		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
		FinalScoreBoardWidget->BindPlayerEntry();
		FinalScoreBoardWidget->AddToViewport();
	}
}

void APGHUD::InitPauseMenuWidget()
{
	if (!PauseMenuWidgetClass || (MainMenuWidget && MainMenuWidget->IsInViewport()) ||
		(ScoreBoardWidget && ScoreBoardWidget->IsInViewport()) || (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport()))
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UPGPauseMenuWidget>(PC, PauseMenuWidgetClass);
	}

	if (PauseMenuWidget && !PauseMenuWidget->IsInViewport())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;

		PauseMenuWidget->AddToViewport();
	}
}

/*
* 홀딩 진행률 디스플레이 위젯 업데이트
*/
void APGHUD::UpdateInteractionProgress(float Progress)
{
	if (!InteractionProgressWidget)
	{
		InteractionProgressWidget = CreateWidget<UPGInteractionProgressWidget>(GetOwningPlayerController(), InteractionProgressWidgetClass);
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

/*
* 실패 메시지 디스플레이
*/
void APGHUD::DisplayInteractionFailedMessage(const FText& Message, float Duration)
{
	if (!MessageManagerWidget)
	{
		MessageManagerWidget = CreateWidget<UPGMessageManagerWidget>(GetOwningPlayerController(), MessageManagerWidgetClass);
	}

	if (MessageManagerWidget)
	{
		MessageManagerWidget->ShowFailureMessage(Message, Duration);
	}
}

void APGHUD::DisplayJumpscare(UTexture2D* JumpscareTexture)
{
	if (!GetOwningPlayerController() || !JumpscareWidgetClass || !JumpscareTexture)
	{
		return;
	}

	if (!JumpscareWidget)
	{
		JumpscareWidget = CreateWidget<UPGJumpscareWidget>(GetOwningPlayerController(), JumpscareWidgetClass);
	}

	if (JumpscareWidget && !JumpscareWidget->IsInViewport())
	{
		JumpscareWidget->Init(JumpscareTexture);
		JumpscareWidget->AddToViewport(100);
	}
}

void APGHUD::ClearViewport()
{
	if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
	{
		Viewport->RemoveAllViewportWidgets();
	}
}
