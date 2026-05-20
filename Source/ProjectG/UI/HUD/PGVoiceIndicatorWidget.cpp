// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGVoiceIndicatorWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGGameUserSettings.h"
#include "Player/PGPlayerController.h"
#include "Player/PGLobbyPlayerController.h"


void UPGVoiceIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	float Amplitude = 0.0f;
	if (APGPlayerCharacter* Player = GetOwningPlayerPawn<APGPlayerCharacter>())
	{
		Amplitude = Player->GetCurrentVoiceAmplitude();
	}

	const bool bPushToTalk = IsPushToTalkEnabled();
	if (!bKeyHintInitialized || bPushToTalk != bCachedPushToTalk)
	{
		bCachedPushToTalk = bPushToTalk;
		ApplyKeyHintVisibility(bPushToTalk);
		bKeyHintInitialized = true;
	}

	// 라디오 아이콘 -> Opacity + Color
	if (RadioIcon)
	{
		float TargetOpacity;
		if (Amplitude >= TalkingThreshold)
		{
			TargetOpacity = ActiveOpacity;
		}
		else
		{
			TargetOpacity = IsMicReady() ? ReadyOpacity : InactiveOpacity;
		}

		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, InDeltaTime, FadeSpeed);
		RadioIcon->SetRenderOpacity(CurrentOpacity);

		const FLinearColor TargetColor = (Amplitude >= DangerThreshold) ? DangerColor : NormalColor;
		RadioIcon->SetColorAndOpacity(TargetColor);
	}

	// 디버깅용 ProgressBar
	if (VoiceAmplitudeBar)
	{
		const float Speed = (Amplitude > DisplayAmplitude) ? 9.0f : 5.0f;
		DisplayAmplitude = FMath::FInterpTo(DisplayAmplitude, Amplitude, InDeltaTime, Speed);

		const float DisplayPercent = FMath::Clamp(DisplayAmplitude / 0.25f, 0.0f, 1.0f);
		VoiceAmplitudeBar->SetPercent(DisplayPercent);
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

void UPGVoiceIndicatorWidget::ApplyKeyHintVisibility(bool bPushToTalk)
{
	if (KeyHintIcon)
	{
		KeyHintIcon->SetVisibility(bPushToTalk ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
