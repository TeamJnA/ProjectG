// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGHelperExitEntryWidget.h"
#include "Type/PGHelperTypes.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"


void UPGHelperExitEntryWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		World->GetTimerManager().ClearTimer(IconAppearTimerHandle);
	}

	Super::NativeDestruct();
}

void UPGHelperExitEntryWidget::SetEntry(int32 InSpeciesKey, const FPGHelperEntryRow& Row, const TSet<FName>& UnlockedItemIds, bool bDepleted)
{
	SpeciesKey = InSpeciesKey;
	bIsDepleted = bDepleted;
	bIntroComplete = false;

	FullDisplayName = Row.DisplayName.ToString();
	TypewriterIndex = 0;
	IconAppearIndex = 0;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		World->GetTimerManager().ClearTimer(IconAppearTimerHandle);
	}

	if (NameText)
	{
		NameText->SetText(FText::GetEmpty());
	}

	SpawnedIconImages.Reset();
	SpawnedIconItemIds.Reset();

	if (ItemIconBox)
	{
		ItemIconBox->ClearChildren();

		for (const FPGHelperItemIcon& Item : Row.RequiredItemIcons)
		{
			if (!Item.Icon)
			{
				continue;
			}

			UImage* IconImage = NewObject<UImage>(this);

			FSlateBrush Brush;
			Brush.SetResourceObject(Item.Icon);
			Brush.ImageSize = Item.Size;
			Brush.DrawAs = ESlateBrushDrawType::Image;
			IconImage->SetBrush(Brush);
			IconImage->SetDesiredSizeOverride(Item.Size);

			const bool bUnlocked = UnlockedItemIds.Contains(Item.ItemId);
			IconImage->SetColorAndOpacity(bUnlocked ? UnlockedColor : LockedColor);
			IconImage->SetRenderOpacity(0.0f);

			if (UHorizontalBoxSlot* IconImageSlot = ItemIconBox->AddChildToHorizontalBox(IconImage))
			{
				IconImageSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
				IconImageSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
				IconImageSlot->SetPadding(FMargin(2.0f, 0.0f));
			}
			SpawnedIconImages.Add(IconImage);
			SpawnedIconItemIds.Add(Item.ItemId);
		}
	}

	if (DepletedLine)
	{
		DepletedLine->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPGHelperExitEntryWidget::PlayIntroAnim()
{
	if (bIntroComplete)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (TypewriterStepInterval > 0.0f && !FullDisplayName.IsEmpty())
	{
		World->GetTimerManager().SetTimer(TypewriterTimerHandle, this, &UPGHelperExitEntryWidget::TypewriterStep, TypewriterStepInterval, true);
	}
	else if (NameText)
	{
		NameText->SetText(FText::FromString(FullDisplayName));
	}

	const float TextTotal = FullDisplayName.Len() * TypewriterStepInterval;
	const float IconStart = TextTotal + IconStartDelay;
	World->GetTimerManager().SetTimer(IconAppearTimerHandle, this, &UPGHelperExitEntryWidget::IconAppearStep, IconStepInterval, true, IconStart);
}

void UPGHelperExitEntryWidget::UpdateInPlace(const TSet<FName>& UnlockedItemIds, bool bDepleted)
{
	bIsDepleted = bDepleted;

	const int32 IconCount = FMath::Min(SpawnedIconImages.Num(), SpawnedIconItemIds.Num());
	for (int32 i = 0; i < IconCount; ++i)
	{
		UImage* Img = SpawnedIconImages[i];
		if (!Img)
		{
			continue;
		}

		const bool bUnlocked = UnlockedItemIds.Contains(SpawnedIconItemIds[i]);
		Img->SetColorAndOpacity(bUnlocked ? UnlockedColor : LockedColor);
	}

	if (DepletedLine)
	{
		DepletedLine->SetVisibility(bIsDepleted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UPGHelperExitEntryWidget::TypewriterStep()
{
	++TypewriterIndex;
	if (TypewriterIndex >= FullDisplayName.Len())
	{
		if (NameText)
		{
			NameText->SetText(FText::FromString(FullDisplayName));
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		}

		return;
	}

	if (NameText)
	{
		NameText->SetText(FText::FromString(FullDisplayName.Left(TypewriterIndex)));
	}
}

void UPGHelperExitEntryWidget::IconAppearStep()
{
	if (SpawnedIconImages.IsValidIndex(IconAppearIndex))
	{
		if (UImage* Img = SpawnedIconImages[IconAppearIndex])
		{
			Img->SetRenderOpacity(1.0f);
		}
		++IconAppearIndex;

		if (IconAppearIndex >= SpawnedIconImages.Num())
		{
			CompleteIntro();
		}
	}
	else
	{
		CompleteIntro();
	}
}

void UPGHelperExitEntryWidget::CompleteIntro()
{
	if (bIsDepleted && DepletedLine)
	{
		DepletedLine->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	bIntroComplete = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IconAppearTimerHandle);
	}
}
