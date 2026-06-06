// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGVoiceIndicatorWidget.h"
#include "Components/Image.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGSoundManagerComponent.h"
#include "Player/PGGameUserSettings.h"
#include "Player/PGPlayerController.h"
#include "Player/PGLobbyPlayerController.h"


void UPGVoiceIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

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

	if (MicModeIcon)
	{
		const float TargetMicOpacity = IsMicReady() ? MicActiveOpaciity : MicInactiveOpacity;
		CurrentMicOpacity = FMath::FInterpTo(CurrentMicOpacity, TargetMicOpacity, InDeltaTime, MicOpacityInterpSpeed);
		MicModeIcon->SetRenderOpacity(CurrentMicOpacity);
	}
}

bool UPGVoiceIndicatorWidget::IsPushToTalkEnabled() const
{
	if (const UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		return Settings->IsPushToTalk();
	}
	return false;
}

bool UPGVoiceIndicatorWidget::IsMicReady() const
{
	if (!IsPushToTalkEnabled())
	{
		return true;
	}

	APlayerController* PC = GetOwningPlayer();
	if (APGPlayerController* PGPC = Cast<APGPlayerController>(PC))
	{
		return PGPC->IsPushToTalkActive();
	}
	else if (APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(PC))
	{
		return LobbyPC->IsPushToTalkActive();
	}
	return false;
}
