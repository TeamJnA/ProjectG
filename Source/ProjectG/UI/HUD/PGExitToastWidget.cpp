// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGExitToastWidget.h"
#include "Components/TextBlock.h"


void UPGExitToastWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetRenderOpacity(0.0f);
}

void UPGExitToastWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterDelayHandle);
		World->GetTimerManager().ClearTimer(TypewriterStepHandle);
	}

	Super::NativeDestruct();
}

void UPGExitToastWidget::ShowToast()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterDelayHandle);
		World->GetTimerManager().ClearTimer(TypewriterStepHandle);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(1.0f);

	if (ExitMessageText)
	{
		ExitMessageText->SetText(FText::GetEmpty());
	}

	if (FadeAnim)
	{
		StopAnimation(FadeAnim);
		PlayAnimation(FadeAnim);
	}

	StartTypewriter();
}

void UPGExitToastWidget::StartTypewriter()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TypewriterDelayHandle, this, &UPGExitToastWidget::OnTypewriterDelay, TypewriterStartDelay, false);
	}
}

void UPGExitToastWidget::OnTypewriterDelay()
{
	FullString = ExitToastText.ToString();
	CharIndex = 0;
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TypewriterStepHandle, this, &UPGExitToastWidget::OnTypewriterStep, TypewriterStepInterval, true);
	}
}

void UPGExitToastWidget::OnTypewriterStep()
{
	++CharIndex;
	if (CharIndex >= FullString.Len())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TypewriterStepHandle);
		}

		if (ExitMessageText)
		{
			ExitMessageText->SetText(FText::FromString(FullString));
		}
	}
	else if (ExitMessageText)
	{
		ExitMessageText->SetText(FText::FromString(FullString.Left(CharIndex)));
	}
}

void UPGExitToastWidget::DismissToast()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterDelayHandle);
		World->GetTimerManager().ClearTimer(TypewriterStepHandle);
	}

	if (FadeAnim)
	{
		StopAnimation(FadeAnim);
		PlayAnimation(FadeAnim, 0.0f, 1, EUMGSequencePlayMode::Reverse);
	}
	else
	{
		SetRenderOpacity(0.0f);
	}
}

void UPGExitToastWidget::HideImmediate()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterDelayHandle);
		World->GetTimerManager().ClearTimer(TypewriterStepHandle);
	}

	if (FadeAnim && IsAnimationPlaying(FadeAnim))
	{
		StopAnimation(FadeAnim);
	}

	SetRenderOpacity(0.0f);
}
