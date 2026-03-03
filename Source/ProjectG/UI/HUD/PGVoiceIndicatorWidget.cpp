// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGVoiceIndicatorWidget.h"
#include "Components/ProgressBar.h"
#include "Character/PGPlayerCharacter.h"


void UPGVoiceIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!VoiceAmplitudeBar)
	{
		return;
	}

	float TargetAmplitude = 0.0f;
	if (APGPlayerCharacter* Player = GetOwningPlayerPawn<APGPlayerCharacter>())
	{
		TargetAmplitude = Player->GetCurrentVoiceAmplitude();
	}

	const float Speed = (TargetAmplitude > DisplayAmplitude) ? 15.0f : 5.0f;
	DisplayAmplitude = FMath::FInterpTo(DisplayAmplitude, TargetAmplitude, InDeltaTime, Speed);
	
	const float DisplayPercent = FMath::Clamp(DisplayAmplitude / 0.2f, 0.0f, 1.0f);
	VoiceAmplitudeBar->SetPercent(DisplayPercent);
}
