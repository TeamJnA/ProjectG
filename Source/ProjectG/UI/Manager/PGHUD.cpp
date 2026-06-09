// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Manager/PGHUD.h"

#include "UI/Menu/PGMainMenuWidget.h"
#include "UI/Menu/PGPauseMenuWidget.h"

#include "UI/PlayerEntry/Lobby/PGLobbyWidget.h"
#include "UI/PlayerEntry/Spectator/PGSpectatorWidget.h"
#include "UI/PlayerEntry/ScoreBoard/PGScoreBoardWidget.h"
#include "UI/PlayerEntry/ScoreBoard/PGFinalScoreBoardWidget.h"

#include "UI/HUD/PGIndicatorContainerWidget.h"
#include "UI/HUD/PGAlertContainerWidget.h"
#include "UI/HUD/PGInventoryWidget.h"
#include "UI/HUD/PGMessageManagerWidget.h"
#include "UI/HUD/PGInteractionProgressWidget.h"
#include "UI/HUD/PGCameraWidget.h"
#include "UI/HUD/PGCrosshairWidget.h"
#include "UI/HUD/PGBackgroundBlurWidget.h"

#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Type/PGHelperTypes.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"


APGHUD::APGHUD()
{
}

void APGHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExitBindRetryHandle);
		World->GetTimerManager().ClearTimer(CapturedSubBindRetryHandle);
	}

	UnbindExits();

	if (APGPlayerState* PS = SubscribedPS.Get())
	{
		PS->OnCapturedSubjectsChanged.RemoveDynamic(this, &APGHUD::HandleCapturedSubjectsChanged);
	}

	Super::EndPlay(EndPlayReason);
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

	if (!IndicatorContainerWidget)
	{
		IndicatorContainerWidget = CreateWidget<UPGIndicatorContainerWidget>(PC, IndicatorContainerWidgetClass);
	}

	if (IndicatorContainerWidget && !IndicatorContainerWidget->IsInViewport())
	{
		IndicatorContainerWidget->Init();
		IndicatorContainerWidget->AddToViewport();
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		if (GS->GetCurrentGameState() != EGameState::Lobby)
		{
			if (!AlertContainerWidget)
			{
				AlertContainerWidget = CreateWidget<UPGAlertContainerWidget>(PC, AlertContainerWidgetClass);
			}

			if (AlertContainerWidget && !AlertContainerWidget->IsInViewport())
			{
				AlertContainerWidget->AddToViewport(10);
			}
		}
	}

	// for test
	//if (!AlertContainerWidget)
	//{
	//	AlertContainerWidget = CreateWidget<UPGAlertContainerWidget>(PC, AlertContainerWidgetClass);
	//}
	//if (AlertContainerWidget && !AlertContainerWidget->IsInViewport())
	//{
	//	AlertContainerWidget->AddToViewport(10);
	//}

	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UPGInventoryWidget>(PC, InventoryWidgetClass);
	}

	if (InventoryWidget && !InventoryWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("APGHUD::Init: InventoryWidget created successfully."));
		InventoryWidget->SetHelperGuideAvailable(AlertContainerWidget != nullptr);
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

	TryBindExits();
	TrySubscribeCapturedSubjects();
}

void APGHUD::TryBindExits()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 BoundCount = 0;
	for (TActorIterator<APGExitPointBase> It(World); It; ++It)
	{
		APGExitPointBase* Exit = *It;
		if (!Exit)
		{
			continue;
		}

		Exit->OnExitLockStateChanged.AddUniqueDynamic(this, &APGHUD::HandleExitLockStateChanged);
		HUDSubscribedExits.AddUnique(Exit);
		++BoundCount;
	}

	if (BoundCount == 0 && ExitBindRetries < 10)
	{
		++ExitBindRetries;
		World->GetTimerManager().SetTimer(ExitBindRetryHandle, this, &APGHUD::TryBindExits, 0.5f, false);
	}
}

void APGHUD::UnbindExits()
{
	for (const TWeakObjectPtr<APGExitPointBase>& WeakExit : HUDSubscribedExits)
	{
		if (APGExitPointBase* Exit = WeakExit.Get())
		{
			Exit->OnExitLockStateChanged.RemoveDynamic(this, &APGHUD::HandleExitLockStateChanged);
		}
	}
	HUDSubscribedExits.Reset();
}

void APGHUD::HandleExitLockStateChanged(APGExitPointBase* ExitActor)
{
	if (!ExitActor || !HelperCatalogTable)
	{
		return;
	}

	const int32 SpeciesKey = ExitActor->GetLinkedSpeciesKey();
	if (SpeciesKey == 0)
	{
		return;
	}

	static const FString Ctx(TEXT("APGHUD::HandleExitLockStateChanged"));
	const FName RowName(*FString::FromInt(SpeciesKey));
	const FPGHelperEntryRow* Row = HelperCatalogTable->FindRow<FPGHelperEntryRow>(RowName, Ctx, false);
	if (!Row)
	{
		return;
	}

	// 발견 판정: bDefaultVisible 이거나, 로컬 PlayerState가 이 SpeciesKey를 캡처한 적 있음
	bool bDiscovered = Row->bDefaultVisible;
	if (!bDiscovered)
	{
		if (APlayerController* PC = GetOwningPlayerController())
		{
			if (APGPlayerState* PS = PC->GetPlayerState<APGPlayerState>())
			{
				for (int32 ID : PS->GetCapturedIDs())
				{
					if (PhotoID::GetSpeciesKey(ID) == SpeciesKey)
					{
						bDiscovered = true;
						break;
					}
				}
			}
		}
	}

	if (bDiscovered)
	{
		DisplayExitToast();

		if (AlertContainerWidget)
		{
			AlertContainerWidget->NotifyHelperExitLockChanged();
		}
	}
}

void APGHUD::TrySubscribeCapturedSubjects()
{
	if (SubscribedPS.IsValid())
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	APGPlayerState* PS = PC ? PC->GetPlayerState<APGPlayerState>() : nullptr;
	if (PS)
	{
		PS->OnCapturedSubjectsChanged.AddUniqueDynamic(this, &APGHUD::HandleCapturedSubjectsChanged);
		SubscribedPS = PS;
		return;
	}

	if (CapturedSubBindRetries < 10)
	{
		++CapturedSubBindRetries;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(CapturedSubBindRetryHandle, this, &APGHUD::TrySubscribeCapturedSubjects, 0.5f, false);
		}
	}
}

void APGHUD::HandleCapturedSubjectsChanged()
{
	if (AlertContainerWidget)
	{
		AlertContainerWidget->NotifyHelperCapturedChanged();
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

		CloseHelperWidgetIfOpen();

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

// 탈출/사망
void APGHUD::ForceCleanupHUD()
{
	// 카메라 위젯 즉시 제거
	if (CameraWidget && CameraWidget->IsInViewport())
	{
		CameraWidget->RemoveFromParent();
	}

	// 일반 HUD 제거
	if (IndicatorContainerWidget)
	{
		IndicatorContainerWidget->SetVisibility(ESlateVisibility::Visible);
		if (IndicatorContainerWidget->IsInViewport())
		{
			IndicatorContainerWidget->RemoveFromParent();
		}
	}

	if (AlertContainerWidget)
	{
		AlertContainerWidget->ResetForCleanup();
		if (AlertContainerWidget->IsInViewport())
		{
			AlertContainerWidget->RemoveFromParent();
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
	if (IndicatorContainerWidget && IndicatorContainerWidget->IsInViewport())
	{
		IndicatorContainerWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (AlertContainerWidget)
	{
		AlertContainerWidget->OnEnterCameraMode();
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
	if (IndicatorContainerWidget)
	{
		IndicatorContainerWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (AlertContainerWidget)
	{
		AlertContainerWidget->OnExitCameraMode();
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

void APGHUD::BeginExitCameraTransition()
{
	if (AlertContainerWidget)
	{
		AlertContainerWidget->OnExitCameraMode_EnemyToast();
	}
}

void APGHUD::SetPhotoAlertVisible(bool bVisible)
{
	if (AlertContainerWidget)
	{
		AlertContainerWidget->SetPhotoAlertActive(bVisible);
	}
}

void APGHUD::SetCrosshairVisible(bool bVisible)
{
	if (CrosshairWidget)
	{
		CrosshairWidget->SetCrosshairVisible(bVisible);
	}
}

void APGHUD::ToggleHelperWidget()
{
	if (!CanOpenHelperWidget())
	{
		return;
	}

	AlertContainerWidget->OpenHelper();
}

bool APGHUD::CanOpenHelperWidget() const
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return false;
	}

	if (APGPlayerState* PS = PC->GetPlayerState<APGPlayerState>())
	{
		if (!PS->IsInGame() || PS->IsSpectating())
		{
			return false;
		}
	}

	if (!AlertContainerWidget)
	{
		return false;
	}

	if (ScoreBoardWidget && ScoreBoardWidget->IsInViewport())
	{
		return false;
	}

	if (FinalScoreBoardWidget && FinalScoreBoardWidget->IsInViewport())
	{
		return false;
	}

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		return false;
	}

	if (CameraWidget && CameraWidget->IsInViewport())
	{
		return false;
	}

	if (AlertContainerWidget && AlertContainerWidget->IsHelperOpen())
	{
		return false;
	}

	return true;
}

void APGHUD::CloseHelperWidgetIfOpen()
{
	if (AlertContainerWidget)
	{
		AlertContainerWidget->CloseHelper();
	}
}

void APGHUD::NotifyNewlyCapturedSpeciesKeys(const TArray<int32>& Keys)
{
	if (!HelperCatalogTable || !EnemyCatalogTable || Keys.IsEmpty())
	{
		return;
	}

	bool bHasHelperUpdate = false;

	for (int32 Key : Keys)
	{
		const FString KeyStr = FString::FromInt(Key);
		if (HelperCatalogTable->FindRow<FPGHelperEntryRow>(*KeyStr, TEXT(""), false))
		{
			bHasHelperUpdate = true;
			continue;
		}

		if (FPGEnemyCatalogRow* EnemyRow = EnemyCatalogTable->FindRow<FPGEnemyCatalogRow>(*KeyStr, TEXT(""), false))
		{
			DisplayEnemyToast(EnemyRow->TooltipText);
		}
	}

	if (bHasHelperUpdate)
	{
		DisplayExitToast();
	}
}

void APGHUD::DisplayExitToast()
{
	if (AlertContainerWidget)
	{
		AlertContainerWidget->RequestExitToast();
	}
}

void APGHUD::DisplayEnemyToast(const FText& TooltipText)
{
	if (AlertContainerWidget)
	{
		AlertContainerWidget->PlayEnemyToast(TooltipText);
	}
}
