// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGVoiceIndicatorWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGVoiceIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	bool IsPushToTalkEnabled() const;
	bool IsMicReady() const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> NoiseBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MicModeIcon;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BarMID;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	FName FillRatioParam = TEXT("FillRatio");

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	float VoiceMaxAmplitude = 0.25f; // 이 이상이면 VoiceMaxLevel

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	float VoiceMaxLevel = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	float MaxDisplayLevel = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	float FallInterpSpeed = 6.0f;

	float DisplayLevel = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Mic")
	float MicActiveOpaciity = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Mic")
	float MicInactiveOpacity = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator|Mic")
	float MicOpacityInterpSpeed = 10.0f;

	float CurrentMicOpacity = 1.0f;
};
