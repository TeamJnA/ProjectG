// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/PGPhotoTypes.h"
#include "PGCameraWidget.generated.h"

class UImage;
class UTextBlock;
class UVerticalBox;
class UMediaPlayer;
class UMediaSource;
class UImage;
class UPGCaptureLogEntryWidget;
class UPanelWidget;
class UProgressBar;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_DELEGATE(FOnCameraTransitionFinished);

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGCameraWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void SetProgress(float Progress);
    void DisplayPhotoResult(const TArray<FPhotoSubjectInfo>& Results, int32 TotalScore);
    void PlayFadeInTransition(FOnCameraTransitionFinished OnFinished);
    void PlayFadeOutTransition(FOnCameraTransitionFinished OnFinished);
    void UpdateZoomIndicator(float ZoomAlpha);
    void UpdateBattery(float Percent);
    void PlayCaptureEffect();
    void UpdateFocusFramePosition(const FVector2D& ScreenPosition);
    void ResetFocusFrame();

    void BeginCapture(UTextureRenderTarget2D* PhotoRT);
    void FeedCaptureLines(const TArray<FCaptureLogLine>& Lines);
    void StartCaptureCooldown(float Duration);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void OnFadeTransitionFinished();

    void StartGlitchPlayback();
    void StopGlitchPlayback();
    float SnapToDisplayStep(float Percent) const;

    void ResetCaptureSequence();

    void OnCooldownStep();

    FOnCameraTransitionFinished CachedOnFinished;
    FWidgetAnimationDynamicEvent FadeAnimDelegate;

    // 0Ä­, 1Ä­, 2Ä­, 3Ä­, 4Ä­, 5Ä­(Ç®)
    UPROPERTY(EditDefaultsOnly, Category = "Battery")
    TArray<float> BatteryDisplaySteps = { 0.0f, 0.11f, 0.36f, 0.61f, 0.86f, 1.0f };

    UPROPERTY(EditDefaultsOnly, Category = "Log")
    TSubclassOf<UPGCaptureLogEntryWidget> CaptureLogEntryClass;

    UPROPERTY()
    TObjectPtr<UPGCaptureLogEntryWidget> CurrentLogEntry;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FadeInTransitionAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FadeOutTransitionAnim;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPanelWidget> CaptureLogRoot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> GlitchOverlayImage;

    UPROPERTY(EditDefaultsOnly, Category = "Glitch")
    TObjectPtr<UMediaPlayer> GlitchMediaPlayer;

    UPROPERTY(EditDefaultsOnly, Category = "Glitch")
    TObjectPtr<UMediaSource> GlitchMediaSource;

    UPROPERTY(EditDefaultsOnly, Category = "Glitch")
    TObjectPtr<UMaterialInterface> GlitchMaterialClass;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> GlitchMID;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> FocusFrame;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ZoomIndicator; 
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> BatteryFill;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> BatteryFrame;

    UPROPERTY(EditDefaultsOnly, Category = "Battery")
    TObjectPtr<UMaterialInterface> BatteryFillMaterialClass;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> BatteryFillMID;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> WhiteFlashOverlay;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> WhiteFlashAnim;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> CaptureCooldownBar;

    FTimerHandle CooldownTimerHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float FocusFrameMaxScale = 1.5f;  // ÁÜ ÃÖ´ë ¹èÀ²

    // »ï°¢ÇüÀÌ ÀÌµ¿ÇÒ ÃÖ´ë ÇÈ¼¿ ¹üÀ§ (Áß¾Ó ±âÁØ ¡¾)
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float ZoomIndicatorMaxOffset = 196.7f;

    float CurrentBatteryPercent = 1.0f;

    float CooldownDuration = 0.0f;
    float CooldownElapsed = 0.0f;
    static constexpr float CooldownStep = 0.02f;
};
