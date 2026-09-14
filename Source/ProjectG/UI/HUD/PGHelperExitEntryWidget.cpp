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
	const TMap<EPGExitItemType, int32>& UnlockedItemIds, bool bDepleted, bool bInRevealed)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		World->GetTimerManager().ClearTimer(RequirementTimerHandle);
	}

	SpeciesKey = InSpeciesKey;
	bIsDepleted = bDepleted;
	bIntroComplete = false;
	bRevealed = bInRevealed;

	RealDisplayName = Row.DisplayName.ToString();
	FullDisplayName = bRevealed ? RealDisplayName : MaskedName.ToString();
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

void UPGHelperExitEntryWidget::UpdateInPlace(const TMap<EPGExitItemType, int32>& UnlockedCounts, bool bDepleted, bool bInRevealed)
{
	const bool bJustRevealed = bInRevealed && !bRevealed;
	bRevealed = bInRevealed;
	bIsDepleted = bDepleted;

	if (bJustRevealed)
	{
		FullDisplayName = RealDisplayName;
		if (bIntroComplete && NameText)
		{
			NameText->SetText(FText::FromString(FullDisplayName));
		}
	}

	RebuildRequirementString(UnlockedCounts);

	if (bIntroComplete && RequirementText)
	{
		RequirementText->SetText(FText::FromString(FullRequirementText));
	}

	if (DepletedLine)
	{
		const bool bShowDepleted = bRevealed && bIsDepleted;
		DepletedLine->SetVisibility(bShowDepleted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UPGHelperExitEntryWidget::RebuildRequirementString(const TMap<EPGExitItemType, int32>& UnlockedCounts)
{
	FullRequirementText.Reset();

	if (!bRevealed)
	{
		FullRequirementText = MaskedRequirement.ToString();
		return;
	}

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
	if (bRevealed && bIsDepleted && DepletedLine)
	{
		DepletedLine->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	bIntroComplete = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RequirementTimerHandle);
	}
}
