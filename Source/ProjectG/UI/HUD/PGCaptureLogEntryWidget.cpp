// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGCaptureLogEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/TextureRenderTarget2D.h"


void UPGCaptureLogEntryWidget::NativeDestruct()
{
	ClearAllTimers();
	Super::NativeDestruct();
}

void UPGCaptureLogEntryWidget::SetThumbnail(UTextureRenderTarget2D* RT)
{
	if (Thumbnail && RT)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(RT);
		Thumbnail->SetBrush(Brush);
	}
	StartThumbnailFadeIn();
}

void UPGCaptureLogEntryWidget::StartThumbnailFadeIn()
{
	ThumbnailAlpha = 0.0f;
	bThumbnailFaded = false;
	if (Thumbnail)
	{
		Thumbnail->SetRenderOpacity(0.0f);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FadeInHandle, this, &UPGCaptureLogEntryWidget::OnThumbnailFadeStep, DriverStep, true);
	}
}

void UPGCaptureLogEntryWidget::OnThumbnailFadeStep()
{
	ThumbnailAlpha += DriverStep / ThumbnailFadeInDuration;
	const float Alpha = FMath::Min(ThumbnailAlpha, 1.0f);
	if (Thumbnail)
	{
		Thumbnail->SetRenderOpacity(Alpha);
	}

	if (Alpha >= 1.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FadeInHandle);
		}
		bThumbnailFaded = true;
		TryStart();
	}
}

void UPGCaptureLogEntryWidget::BeginLog(const TArray<FCaptureLogLine>& InLines)
{
	PendingLines = InLines;
	bHasLines = true;
	bStarted = false;
	LineIndex = 0;
	CharIndex = 0;
	Phase = EPhase::Idle;
	PhaseElapsed = 0.0f;
	TypeAccum = 0.0f;
	ActiveText = nullptr;
	OutgoingText = nullptr;
	CurrentFull.Reset();

	if (LineSlot)
	{
		LineSlot->ClearChildren();
	}

	SetRenderOpacity(1.0f);
	TryStart();
}

void UPGCaptureLogEntryWidget::TryStart()
{
	if (!(bThumbnailFaded && bHasLines && !bStarted))
	{
		return;
	}
	bStarted = true;
	LineIndex = 0;

	if (PendingLines.Num() == 0)
	{
		StartEntryFadeOut();
	}
	else
	{
		StartLineIn();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DriverHandle, this, &UPGCaptureLogEntryWidget::Driver, DriverStep, true);
	}
}

void UPGCaptureLogEntryWidget::StartLineIn()
{
	const FCaptureLogLine& Line = PendingLines[LineIndex];
	ActiveText = MakeLineText(Line);
	CurrentFull = Line.Text.ToString();
	CharIndex = 0;
	TypeAccum = 0.0f;

	ApplyLineState(ActiveText, FVector2D(SlideInOffset, 0.0f), 0.0f);
	Phase = EPhase::LineIn;
	PhaseElapsed = 0.0f;
}

void UPGCaptureLogEntryWidget::StartTransition()
{
	OutgoingText = ActiveText;

	++LineIndex;
	const FCaptureLogLine& Line = PendingLines[LineIndex];
	ActiveText = MakeLineText(Line);
	CurrentFull = Line.Text.ToString();
	CharIndex = 0;
	TypeAccum = 0.0f;

	ApplyLineState(ActiveText, FVector2D(SlideInOffset, 0.0f), 0.0f);
	Phase = EPhase::Transition;
	PhaseElapsed = 0.0f;
}

UTextBlock* UPGCaptureLogEntryWidget::MakeLineText(const FCaptureLogLine& Line)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (Text && LineSlot)
	{
		Text->SetText(FText::GetEmpty());
		Text->SetFont(LineFont);
		Text->SetColorAndOpacity(Line.bValid ? ValidColor : InvalidColor);

		if (UOverlaySlot* OvelaySlot = LineSlot->AddChildToOverlay(Text))
		{
			OvelaySlot->SetHorizontalAlignment(HAlign_Left);
			OvelaySlot->SetVerticalAlignment(VAlign_Center);
		}
	}
	return Text;
}

void UPGCaptureLogEntryWidget::ApplyLineState(UTextBlock* Text, FVector2D Translation, float Opacity)
{
	if (!Text)
	{
		return;
	}

	FWidgetTransform Transform;
	Transform.Translation = Translation;
	Text->SetRenderTransform(Transform);
	Text->SetRenderOpacity(Opacity);
}

void UPGCaptureLogEntryWidget::AdvanceTypewriter()
{
	if (!ActiveText || IsTypingDone())
	{
		return;
	}

	TypeAccum += DriverStep;

	bool bChanged = false;
	while (TypeAccum >= TypeInterval && CharIndex < CurrentFull.Len())
	{
		++CharIndex;
		TypeAccum -= TypeInterval;
		bChanged = true;
	}

	if (bChanged)
	{
		ActiveText->SetText(FText::FromString(CurrentFull.Left(CharIndex)));
	}
}

void UPGCaptureLogEntryWidget::StartEntryFadeOut()
{
	Phase = EPhase::EntryFadeOut;
	PhaseElapsed = 0.0f;
}

void UPGCaptureLogEntryWidget::Driver()
{
	PhaseElapsed += DriverStep;

	switch (Phase)
	{
		case UPGCaptureLogEntryWidget::EPhase::LineIn:
		{
			const float SlideProgress = FMath::Clamp(PhaseElapsed / SlideDuration, 0.0f, 1.0f);
			const float Eased = SlideProgress * SlideProgress * (3.0f - 2.0f * SlideProgress);
			ApplyLineState(ActiveText, FVector2D(SlideInOffset * (1.0f - Eased), 0.0f), Eased);
			AdvanceTypewriter();

			if (SlideProgress >= 1.0f && IsTypingDone())
			{
				Phase = EPhase::Hold;
				PhaseElapsed = 0.0f;
			}

			break;
		}
		case UPGCaptureLogEntryWidget::EPhase::Hold:
		{
			if (PhaseElapsed >= LineHoldDuration)
			{
				if (LineIndex + 1 < PendingLines.Num())
				{
					StartTransition();
				}
				else
				{
					StartEntryFadeOut();
				}
			}

			break;
		}
		case UPGCaptureLogEntryWidget::EPhase::Transition:
		{
			const float SlideProgress = FMath::Clamp(PhaseElapsed / SlideDuration, 0.0f, 1.0f);
			const float Eased = SlideProgress * SlideProgress * (3.0f - 2.0f * SlideProgress);
			ApplyLineState(OutgoingText, FVector2D(0.0f, -SlideOutOffset * Eased), 1.0f - Eased);
			ApplyLineState(ActiveText, FVector2D(SlideInOffset * (1.0f - Eased), 0.0f), Eased);
			AdvanceTypewriter();

			if (SlideProgress >= 1.0f && OutgoingText)
			{
				OutgoingText->RemoveFromParent();
				OutgoingText = nullptr;
			}

			if (SlideProgress >= 1.0f && IsTypingDone())
			{
				Phase = EPhase::Hold;
				PhaseElapsed = 0.0f;
			}

			break;
		}
		case UPGCaptureLogEntryWidget::EPhase::EntryFadeOut:
		{
			const float FadeAlpha = FMath::Clamp(1.0f - PhaseElapsed / FadeOutDuration, 0.0f, 1.0f);
			SetRenderOpacity(FadeAlpha);
			if (FadeAlpha <= 0.0f)
			{
				ClearAllTimers();
				RemoveFromParent();
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void UPGCaptureLogEntryWidget::ForceRemove()
{
	ClearAllTimers();
	RemoveFromParent();
}

void UPGCaptureLogEntryWidget::ClearAllTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeInHandle);
		World->GetTimerManager().ClearTimer(DriverHandle);
	}
}
