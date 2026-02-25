// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPlayerVoiceEntry.generated.h"

class UImage;
class UTextBlock;
class USlider;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPlayerVoiceEntry : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    void SetupEntry(APlayerState* InPlayerState);
    APlayerState* GetPlayerState() const { return PlayerStateRef.Get(); }

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> PlayerAvatar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> VolumeSlider;

private:
    UFUNCTION()
    void OnVolumeSliderChanged(float NewValue);

    UPROPERTY()
    TWeakObjectPtr<APlayerState> PlayerStateRef;

    bool bIsLoadingSettings = false;
};
