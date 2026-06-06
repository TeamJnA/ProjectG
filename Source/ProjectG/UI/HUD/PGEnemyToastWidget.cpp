// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGEnemyToastWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"


void UPGEnemyToastWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ToastAnim)
	{
		FWidgetAnimationDynamicEvent Finished;
		Finished.BindDynamic(this, &UPGEnemyToastWidget::OnToastAnimFinished);
		BindToAnimationFinished(ToastAnim, Finished);
	}
}

void UPGEnemyToastWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterDelayHandle);
		World->GetTimerManager().ClearTimer(TypewriterStepHandle);
		World->GetTimerManager().ClearTimer(TransitionRestoreHandle);
	}

	Super::NativeDestruct();
}

void UPGEnemyToastWidget::PlayToast(const FText& Text)
{
	if (bIsAnimating)
	{
		PendingTexts.Add(Text);
		return;
	}

	PlayInternal(Text);
}

void UPGEnemyToastWidget::PlayInternal(const FText& Text)
{
	if (!ToastAnim)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterDelayHandle);
		World->GetTimerManager().ClearTimer(TypewriterStepHandle);
	}

	if (EnemyMessageText)
	{
		EnemyMessageText->SetText(FText::GetEmpty());
	}

	FullString = Text.ToString();
	CharIndex = 0;
	bIsAnimating = true;

	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(1.0f);
	PlayAnimation(ToastAnim);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TypewriterDelayHandle, this, &UPGEnemyToastWidget::OnTypewriterDelay, TypewriterStartDelay, false);
	}
}

void UPGEnemyToastWidget::OnTypewriterDelay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TypewriterStepHandle, this, &UPGEnemyToastWidget::OnTypewriterStep, TypewriterStepInterval, true);
	}
}

void UPGEnemyToastWidget::OnTypewriterStep()
{
	++CharIndex;
	if (CharIndex >= FullString.Len())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TypewriterStepHandle);
		}

		if (EnemyMessageText)
		{
			EnemyMessageText->SetText(FText::FromString(FullString));
		}
	}
	else if (EnemyMessageText)
	{
		EnemyMessageText->SetText(FText::FromString(FullString.Left(CharIndex)));
	}
}

void UPGEnemyToastWidget::OnToastAnimFinished()
{
	bIsAnimating = false;
	if (PendingTexts.Num() > 0)
	{
		const FText Next = PendingTexts[0];
		PendingTexts.RemoveAt(0);
		PlayInternal(Next);
	}
}

void UPGEnemyToastWidget::HideBrieflyForTransition()
{
	if (!bIsAnimating)
	{
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TransitionRestoreHandle);
		World->GetTimerManager().SetTimer(TransitionRestoreHandle, this, &UPGEnemyToastWidget::RestoreAfterTransition, TransitionHideDuration, false);
	}
}

void UPGEnemyToastWidget::RestoreAfterTransition()
{
	if (bIsAnimating)
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}