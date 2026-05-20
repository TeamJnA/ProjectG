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
#include "UI/HUD/PGInteractionProgressWidget.h"
#include "UI/HUD/PGVoiceIndicatorWidget.h"
#include "UI/HUD/PGCameraWidget.h"
#include "UI/HUD/PGCrosshairWidget.h"
#include "UI/HUD/PGPhotoAlertWidget.h"
#include "UI/HUD/PGBackgroundBlurWidget.h"

#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGInventoryComponent.h"
#include "Game/PGGameState.h"

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

	if (!InteractionProgressWidget)
	{
		InteractionProgressWidget = CreateWidget<UPGInteractionProgressWidget>(PC, InteractionProgressWidgetClass);
	}

	if (!PhotoAlertWidget)
	{
		PhotoAlertWidget = CreateWidget<UPGPhotoAlertWidget>(PC, PhotoAlertWidgetClass);
	}

	if (PhotoAlertWidget && !PhotoAlertWidget->IsInViewport())
	{
		PhotoAlertWidget->AddToViewport();
	}

	if (!CrosshairWidget)
	{
		CrosshairWidget = CreateWidget<UPGCrosshairWidget>(PC, CrosshairWidgetClass);
	}

	if (CrosshairWidget && !CrosshairWidget->IsInViewport())
	{
		CrosshairWidget->AddToViewport();
	}

	if (!BackgroundBlurWidget)
	{
		BackgroundBlurWidget = CreateWidget<UPGBackgroundBlurWidget>(PC, BackgroundBlurWidgetClass);
	}

	if (BackgroundBlurWidget && !BackgroundBlurWidget->IsInViewport())
	{
		BackgroundBlurWidget->AddToViewport(-1);
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
void APGHUD::DisplayInteractionFailedIcon(UMaterialInterface* Icon, FVector2D IconSize, float Duration, bool bAffectInteractPrompt)
{
	if (!MessageManagerWidget)
	{
		MessageManagerWidget = CreateWidget<UPGMessageManagerWidget>(GetOwningPlayerController(), MessageManagerWidgetClass);
	}

	if (MessageManagerWidget)
	{
		MessageManagerWidget->ShowFailureIcon(Icon, IconSize, Duration, bAffectInteractPrompt);
	}
}

void APGHUD::ClearViewport()
{
	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		if (GS->GetCurrentGameState() == EGameState::Lobby)
		{
			return;
		}
	}

	if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
	{
		Viewport->RemoveAllViewportWidgets();
	}
}

void APGHUD::ForceCleanupHUD()
{
	// 카메라 위젯 즉시 제거
	if (CameraWidget && CameraWidget->IsInViewport())
	{
		CameraWidget->RemoveFromParent();
	}

	// 일반 HUD 제거
	if (AttributeWidget)
	{
		AttributeWidget->SetVisibility(ESlateVisibility::Visible);
		if (AttributeWidget->IsInViewport())
		{
			AttributeWidget->RemoveFromParent();
		}
	}

	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		if (InventoryWidget->IsInViewport())
		{
			InventoryWidget->RemoveFromParent();
		}
	}

	if (MessageManagerWidget)
	{
		MessageManagerWidget->SetVisibility(ESlateVisibility::Visible);
		if (MessageManagerWidget->IsInViewport())
		{
			MessageManagerWidget->RemoveFromParent();
		}
	}

	if (InteractionProgressWidget)
	{
		InteractionProgressWidget->SetVisibility(ESlateVisibility::Visible);
		if (InteractionProgressWidget->IsInViewport())
		{
			InteractionProgressWidget->RemoveFromParent();
		}
	}

	if (PhotoAlertWidget)
	{
		PhotoAlertWidget->SetVisibility(ESlateVisibility::Visible);
		if (PhotoAlertWidget->IsInViewport())
		{
			PhotoAlertWidget->RemoveFromParent();
		}
	}

	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
		if (CrosshairWidget->IsInViewport())
		{
			CrosshairWidget->RemoveFromParent();
		}
	}
}

void APGHUD::EnterCameraMode()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		if (GS->GetCurrentGameState() == EGameState::Lobby)
		{
			if (LobbyWidget && LobbyWidget->IsInViewport())
			{
				LobbyWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// 기존 HUD 숨기기
	if (AttributeWidget && AttributeWidget->IsInViewport())
	{
		AttributeWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (MessageManagerWidget && MessageManagerWidget->IsInViewport())
	{
		MessageManagerWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (InteractionProgressWidget && InteractionProgressWidget->IsInViewport())
	{
		InteractionProgressWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (PhotoAlertWidget && PhotoAlertWidget->IsInViewport())
	{
		PhotoAlertWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CrosshairWidget && CrosshairWidget->IsInViewport())
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 카메라 위젯 표시
	if (!CameraWidget)
	{
		CameraWidget = CreateWidget<UPGCameraWidget>(PC, CameraWidgetClass);
	}

	if (CameraWidget && !CameraWidget->IsInViewport())
	{
		CameraWidget->AddToViewport();
	}
}

void APGHUD::ExitCameraMode()
{
	// 카메라 위젯 제거
	if (CameraWidget && CameraWidget->IsInViewport())
	{
		CameraWidget->RemoveFromParent();
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		if (GS->GetCurrentGameState() == EGameState::Lobby)
		{
			if (LobbyWidget)
			{
				LobbyWidget->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				InitLobbyWidget();
			}
		}
	}

	// 기존 HUD 복원
	if (AttributeWidget)
	{
		AttributeWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (MessageManagerWidget)
	{
		MessageManagerWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (InteractionProgressWidget)
	{
		InteractionProgressWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (PhotoAlertWidget)
	{
		PhotoAlertWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void APGHUD::UpdateCameraProgress(float Progress)
{
	if (CameraWidget)
	{
		CameraWidget->SetProgress(Progress);
	}
}

void APGHUD::DisplayPhotoResult(const TArray<FPhotoSubjectInfo>& Results, int32 TotalScore)
{
	if (CameraWidget)
	{
		CameraWidget->DisplayPhotoResult(Results, TotalScore);
	}
}

void APGHUD::SetPhotoAlertVisible(bool bVisible)
{
	if (!PhotoAlertWidget)
	{
		return;
	}

	if (bVisible)
	{
		PhotoAlertWidget->StartBlinking();
	}
	else
	{
		PhotoAlertWidget->StopBlinking();
	}
}

void APGHUD::SetCrosshairVisible(bool bVisible)
{
	if (CrosshairWidget)
	{
		CrosshairWidget->SetCrosshairVisible(bVisible);
	}
}
