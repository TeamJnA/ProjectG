// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Type/PGPhotoTypes.h"
#include "PGHUD.generated.h"

class UPGIndicatorContainerWidget;
class UPGAlertContainerWidget;
class UPGInventoryWidget;
class UPGInventoryComponent;
class UPGMessageManagerWidget;
class UPGInteractionProgressWidget;
class UPGCameraWidget;
class UPGCrosshairWidget;
class APGExitPointBase;
class UPGBackgroundBlurWidget;
class UPGMainMenuWidget;
class UPGLobbyWidget;
class UPGSpectatorWidget;
class UPGScoreBoardWidget;
class UPGFinalScoreBoardWidget;
class UPGPauseMenuWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGHUD : public AHUD
{
	GENERATED_BODY()

public:
	void Init();
	void InitMainMenuWidget();
	void InitLobbyWidget();
	void InitSpectatorWidget();
	void InitScoreBoardWidget();
	void InitFinalScoreBoardWidget();
	void InitPauseMenuWidget();
	void UpdateInteractionProgress(float Progress);
	void DisplayInteractionFailedIcon(UMaterialInterface* Icon, FVector2D IconSize, float Duration, bool bAffectInteractPrompt = true);
	void SetCrosshairVisible(bool bVisible);

	void ClearViewport();
	void ForceCleanupHUD();

	// MessageManagerWidget Getter
	UPGMessageManagerWidget* GetMessageManagerWidget() const { return MessageManagerWidget; }
	// InventoryWidgetGetter
	UPGInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }
	// ScoreBoardWidgetGetter
	UPGScoreBoardWidget* GetScoreBoardWidget() const { return ScoreBoardWidget; }
	// CameraWidgetGetter
	UPGCameraWidget* GetCameraWidget() const { return CameraWidget; }
	// CrosshairWidget Getter
	UPGCrosshairWidget* GetCrosshairWidget() const { return CrosshairWidget; }

	void EnterCameraMode();
	void ExitCameraMode();
	void UpdateCameraProgress(float Progress);
	void DisplayPhotoResult(const TArray<FPhotoSubjectInfo>& Results, int32 TotalScore);

	// -------- AlertWidget(PhotoAlert, Toast, Helper) --------
	void BeginExitCameraTransition();
	void SetPhotoAlertVisible(bool bVisible);
	void ToggleHelperWidget();
	void CloseHelperWidgetIfOpen();
	bool CanOpenHelperWidget() const;
	void NotifyNewlyCapturedSpeciesKeys(const TArray<int32>& Keys);

protected:
	APGHUD();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleExitLockStateChanged(APGExitPointBase* ExitActor);

	void TryBindExits();
	void UnbindExits();

	void DisplayExitToast();
	void DisplayEnemyToast(const FText& TooltipText);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGIndicatorContainerWidget> IndicatorContainerWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGAlertContainerWidget> AlertContainerWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGMessageManagerWidget> MessageManagerWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInteractionProgressWidget> InteractionProgressWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGCameraWidget> CameraWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> HelperCatalogTable;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> EnemyCatalogTable;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGBackgroundBlurWidget> BackgroundBlurWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGMainMenuWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGLobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGSpectatorWidget> SpectatorWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGScoreBoardWidget> ScoreBoardWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGFinalScoreBoardWidget> FinalScoreBoardWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGPauseMenuWidget> PauseMenuWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPGIndicatorContainerWidget> IndicatorContainerWidget;

	UPROPERTY()
	TObjectPtr<UPGAlertContainerWidget> AlertContainerWidget;

	UPROPERTY()
	TObjectPtr<UPGInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UPGMessageManagerWidget> MessageManagerWidget;

	UPROPERTY()
	TObjectPtr<UPGInteractionProgressWidget> InteractionProgressWidget;

	UPROPERTY()
	TObjectPtr<UPGCameraWidget> CameraWidget;

	UPROPERTY()
	TObjectPtr<UPGCrosshairWidget> CrosshairWidget;

	UPROPERTY()
	TObjectPtr<UPGBackgroundBlurWidget> BackgroundBlurWidget;

	UPROPERTY()
	TObjectPtr<UPGMainMenuWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UPGLobbyWidget> LobbyWidget;
	
	UPROPERTY()
	TObjectPtr<UPGSpectatorWidget> SpectatorWidget;

	UPROPERTY()
	TObjectPtr<UPGScoreBoardWidget> ScoreBoardWidget;

	UPROPERTY()
	TObjectPtr<UPGFinalScoreBoardWidget> FinalScoreBoardWidget;

	UPROPERTY()
	TObjectPtr<UPGPauseMenuWidget> PauseMenuWidget;

	TArray<TWeakObjectPtr<APGExitPointBase>> HUDSubscribedExits;

	FTimerHandle ExitBindRetryHandle;

	int32 ExitBindRetries = 0;
};
