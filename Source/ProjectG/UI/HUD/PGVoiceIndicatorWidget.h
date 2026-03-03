// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGVoiceIndicatorWidget.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGVoiceIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> VoiceAmplitudeBar;

private:
	float DisplayAmplitude = 0.0f;
};
