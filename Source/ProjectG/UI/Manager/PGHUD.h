// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Type/PGPhotoTypes.h"
#include "PGHUD.generated.h"

class UPGAttributesWidget;
class UPGInventoryWidget;
class UPGInventoryComponent;
class UPGMessageManagerWidget;
class UPGInteractionProgressWidget;
class UPGVoiceIndicatorWidget;
class UPGCameraWidget;
class UPGCrosshairWidget;
class UPGPhotoAlertWidget;
class UPGBackgroundBlurWidget;
class UPGMainMenuWidget;
class UPGLobbyWidget;
class UPGSpectatorWidget;
class UPGScoreBoardWidget;
class UPGFinalScoreBoardWidget;
class UPGPauseMenuWidget;
class UPGJumpscareWidget;

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
	void DisplayInteractionFailedMessage(const FText& Message, float Duration);
	void DisplayJumpscare(UTexture2D* JumpscareTexture);

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

	void EnterCameraMode();
	void ExitCameraMode();
	void UpdateCameraProgress(float Progress);
	void DisplayPhotoResult(const TArray<FPhotoSubjectInfo>& Results, int32 TotalScore);
	void SetPhotoAlertVisible(bool bVisible);

protected:
	APGHUD();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGAttributesWidget> AttributeWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGMessageManagerWidget> MessageManagerWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInteractionProgressWidget> InteractionProgressWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGVoiceIndicatorWidget> VoiceIndicatorWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGCameraWidget> CameraWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGPhotoAlertWidget> PhotoAlertWidgetClass;

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

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGJumpscareWidget> JumpscareWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPGAttributesWidget> AttributeWidget;

	UPROPERTY()
	TObjectPtr<UPGInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UPGMessageManagerWidget> MessageManagerWidget;

	UPROPERTY()
	TObjectPtr<UPGInteractionProgressWidget> InteractionProgressWidget;

	UPROPERTY()
	TObjectPtr<UPGVoiceIndicatorWidget> VoiceIndicatorWidget;

	UPROPERTY()
	TObjectPtr<UPGCameraWidget> CameraWidget;

	UPROPERTY()
	TObjectPtr<UPGCrosshairWidget> CrosshairWidget;

	UPROPERTY()
	TObjectPtr<UPGPhotoAlertWidget> PhotoAlertWidget;

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

	UPROPERTY()
	TObjectPtr<UPGJumpscareWidget> JumpscareWidget;
};
