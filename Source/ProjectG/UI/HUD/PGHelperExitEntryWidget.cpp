// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGHelperExitEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"


void UPGHelperExitEntryWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		World->GetTimerManager().ClearTimer(RequirementTimerHandle);
	}

	Super::NativeDestruct();
}

void UPGHelperExitEntryWidget::SetEntry(int32 InSpeciesKey, const FPGHelperEntryRow& Row,
	const TMap<EPGExitItemType, int32>& UnlockedItemIds, bool bDepleted)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		World->GetTimerManager().ClearTimer(RequirementTimerHandle);
	}

	SpeciesKey = InSpeciesKey;
	bIsDepleted = bDepleted;
	bIntroComplete = false;

	FullDisplayName = Row.DisplayName.ToString();
	TypewriterIndex = 0;

	if (NameText)
	{
		NameText->SetText(FText::GetEmpty());
	}

	CachedItems = Row.RequiredItems;
	RebuildRequirementString(UnlockedItemIds);
	RequirementIndex = 0;

	if (RequirementText)
	{
		RequirementText->SetText(FText::GetEmpty());
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
	const float ReqStart = TextTotal + RequirementStartDelay;

	if (!FullRequirementText.IsEmpty())
	{
		World->GetTimerManager().SetTimer(RequirementTimerHandle, this,
			&UPGHelperExitEntryWidget::RequirementTypewriterStep, TypewriterStepInterval, true, ReqStart);
	}
	else
	{
		CompleteIntro();
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

void UPGHelperExitEntryWidget::RequirementTypewriterStep()
{
	++RequirementIndex;
	if (RequirementIndex >= FullRequirementText.Len())
	{
		if (RequirementText)
		{
			RequirementText->SetText(FText::FromString(FullRequirementText));
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RequirementTimerHandle);
		}

		CompleteIntro();
		return;
	}

	if (RequirementText)
	{
		RequirementText->SetText(FText::FromString(FullRequirementText.Left(RequirementIndex)));
	}
}

void UPGHelperExitEntryWidget::UpdateInPlace(const TMap<EPGExitItemType, int32>& UnlockedItemIds, bool bDepleted)
{
	bIsDepleted = bDepleted;

	RebuildRequirementString(UnlockedItemIds);

	if (bIntroComplete && RequirementText)
	{
		RequirementText->SetText(FText::FromString(FullRequirementText));
	}

	if (DepletedLine)
	{
		DepletedLine->SetVisibility(bIsDepleted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UPGHelperExitEntryWidget::RebuildRequirementString(const TMap<EPGExitItemType, int32>& UnlockedCounts)
{
	FullRequirementText.Reset();

	for (const FPGHelperRequiredItem& Item : CachedItems)
	{
		const int32 Owned = FMath::Min(UnlockedCounts.FindRef(Item.ItemType), Item.RequiredCount);

		if (!FullRequirementText.IsEmpty())
		{
			FullRequirementText += LINE_TERMINATOR;
		}

		FullRequirementText += FString::Printf(TEXT("%s %d/%d"), *Item.DisplayName.ToString(), Owned, Item.RequiredCount);
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
		World->GetTimerManager().ClearTimer(RequirementTimerHandle);
	}
}
