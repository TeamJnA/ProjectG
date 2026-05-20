// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPhotoAlertWidget.generated.h"

class UImage;
class UPGCameraComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPhotoAlertWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void StartBlinking();
    void StopBlinking();

protected:
    virtual void NativeTick(const FGeometry & MyGeometry, float InDeltaTime) override;

    void RefreshBlinkState();
    UPGCameraComponent* ResolveCameraComponent();

    UPROPERTY(EditAnywhere, Category = "UI")
    FLinearColor NormalTint = FLinearColor::White;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CameraIcon;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> BlinkAnim;

    TWeakObjectPtr<UPGCameraComponent> CachedCam;

    UPROPERTY(EditDefaultsOnly, Category = "Photo")
    float DisabledOpacity = 0.2f;

    bool bBlinkRequested = false;
    bool bCanUseCamera = false;
    bool bAnimActive = false;
};
