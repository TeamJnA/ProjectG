// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGVoiceIndicatorWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Character/PGPlayerCharacter.h"


void UPGVoiceIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	float Amplitude = 0.0f;
	if (APGPlayerCharacter* Player = GetOwningPlayerPawn<APGPlayerCharacter>())
	{
		Amplitude = Player->GetCurrentVoiceAmplitude();
	}

	// 라디오 아이콘 -> Opacity + Color
	if (RadioIcon)
	{
		const float TargetOpacity = (Amplitude >= TalkingThreshold) ? ActiveOpacity : InactiveOpacity;
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

	//float TargetAmplitude = 0.0f;
	//if (APGPlayerCharacter* Player = GetOwningPlayerPawn<APGPlayerCharacter>())
	//{
	//	TargetAmplitude = Player->GetCurrentVoiceAmplitude();
	//}

	//const float Speed = (TargetAmplitude > DisplayAmplitude) ? 9.0f : 5.0f;
	//DisplayAmplitude = FMath::FInterpTo(DisplayAmplitude, TargetAmplitude, InDeltaTime, Speed);
	//
	//const float DisplayPercent = FMath::Clamp(DisplayAmplitude / 0.25f, 0.0f, 1.0f);
	//VoiceAmplitudeBar->SetPercent(DisplayPercent);
}
