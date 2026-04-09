// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPhotoAlertWidget.generated.h"

class UImage;

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
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> CameraIcon;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> BlinkAnim;

    bool bIsBlinking = false;	
};
