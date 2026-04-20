// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGVoiceIndicatorWidget.generated.h"

class UProgressBar;
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

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	FLinearColor DangerColor = FLinearColor::Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> VoiceAmplitudeBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RadioIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	float TalkingThreshold = 0.02f;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	float DangerThreshold = 0.06f;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	float ActiveOpacity = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	float InactiveOpacity = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	float FadeSpeed = 10.0f;

private:
	float DisplayAmplitude = 0.0f;
	float CurrentOpacity = 0.01f;
};
