// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPhotoAlertWidget.generated.h"

class UImage;
class UWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPhotoAlertWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void SetPhotoAlertActive(bool bActive);

    void CollapseForCameraMode();
    void RestoreFromCameraMode();

protected:
    virtual void NativeOnInitialized() override;

    UFUNCTION()
    void HandleSlideAnimFinished();

    void SnapToHiddenState();

    UPROPERTY(EditAnywhere, Category = "UI")
    FLinearColor NormalTint = FLinearColor::White;

    UPROPERTY(EditDefaultsOnly, Category = "Photo")
    FVector2D HiddenTranslation = FVector2D(0.0f, -90.0f);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CameraIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> AlertRoot;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> BlinkAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> SlideInAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> SlideOutAnim;

    bool bShown = false;
};
