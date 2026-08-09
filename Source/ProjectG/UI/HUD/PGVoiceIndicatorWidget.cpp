// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGVoiceIndicatorWidget.h"
#include "Components/Image.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGSoundManagerComponent.h"


void UPGVoiceIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdateNoiseBar(InDeltaTime);
}

void UPGVoiceIndicatorWidget::UpdateNoiseBar(float InDeltaTime)
{
	if (!BarMID)
	{
		if (NoiseBar)
		{
			BarMID = NoiseBar->GetDynamicMaterial();
		}

		if (!BarMID)
		{
			return;
		}
	}

	float VoiceLevel = 0.0f;
	float ActionLevel = 0.0f;

	if (APGPlayerCharacter* Player = GetOwningPlayerPawn<APGPlayerCharacter>())
	{
		const float Amp = Player->GetCurrentVoiceAmplitude();
		VoiceLevel = (VoiceMaxAmplitude > 0.0f) ? FMath::Clamp(Amp / VoiceMaxAmplitude, 0.0f, 1.0f) * VoiceMaxLevel : 0.0f;

		if (UPGSoundManagerComponent* SM = Player->GetSoundManagerComponent())
		{
			ActionLevel = SM->GetCurrentActionNoiseLevel();
		}
	}

	const float TargetLevel = FMath::Max(VoiceLevel, ActionLevel);
	if (TargetLevel >= DisplayLevel)
	{
		DisplayLevel = TargetLevel;
	}
	else
	{
		DisplayLevel = FMath::FInterpTo(DisplayLevel, TargetLevel, InDeltaTime, FallInterpSpeed);
	}

	const float FillRatio = (MaxDisplayLevel > 0.0f) ? FMath::Clamp(DisplayLevel / MaxDisplayLevel, 0.0f, 1.0f) : 0.0f;
	BarMID->SetScalarParameterValue(FillRatioParam, FillRatio);
}

void UPGVoiceIndicatorWidget::NativeDestruct()
{
	if (UPGGameUserSettings* Settings = CachedSettings.Get())
	{
		Settings->OnMicModeChanged.RemoveAll(this);
		Settings->OnMicToggleChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPGVoiceIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		CachedSettings = Settings;
		Settings->OnMicModeChanged.AddUniqueDynamic(this, &UPGVoiceIndicatorWidget::OnMicModeChanged);
		Settings->OnMicToggleChanged.AddUniqueDynamic(this, &UPGVoiceIndicatorWidget::OnMicToggleChanged);
	}

	SetMicIcon();
}

void UPGVoiceIndicatorWidget::OnMicModeChanged(EMicMode InMicMode)
{
	SetMicIcon();
}

void UPGVoiceIndicatorWidget::OnMicToggleChanged(bool bMicToggled)
{
	SetMicIcon();
}

// 마이크 모드 혹은 토글에 따라, 마이크 사용 가능 여부 표시
void UPGVoiceIndicatorWidget::SetMicIcon()
{
	const UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
	if (!Settings)
	{
		return;
	}

	bool bIsReady = Settings->IsMicReady();

	if (MicModeIcon)
	{
		MicModeIcon->SetRenderOpacity(bIsReady ? 1.0f : 0.2f);
	}

	if (MicStopSlash)
	{
		MicStopSlash->SetRenderOpacity(bIsReady ? 0.0f : 1.0f);
	}
}
