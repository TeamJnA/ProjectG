// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGOptionSwitcherWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPGOptionSwitcherWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PrevOptionButton)
	{
		PrevOptionButton->OnClicked.AddUniqueDynamic(this, &UPGOptionSwitcherWidget::OnPrevClicked);
	}

	if (NextOptionButton)
	{
		NextOptionButton->OnClicked.AddUniqueDynamic(this, &UPGOptionSwitcherWidget::OnNextClicked);
	}
}

void UPGOptionSwitcherWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize display without callback
	SetSelectedIndex(CurrentOption, false);
}

void UPGOptionSwitcherWidget::SetSelectedIndex(int32 NewIndex, bool bDoCallback)
{
	if (!Options.IsValidIndex(NewIndex))
	{
		return;
	}

	CurrentOption = NewIndex;
	UpdateDisplay();
	UpdateButtonStates();

	if (bDoCallback)
	{
		OnOptionChanged.Broadcast(CurrentOption);
	}
}

void UPGOptionSwitcherWidget::OnPrevClicked()
{
	if (CurrentOption > 0)
	{
		SetSelectedIndex(CurrentOption - 1, true);
	}
}

void UPGOptionSwitcherWidget::OnNextClicked()
{
	if (CurrentOption < Options.Num() - 1)
	{
		SetSelectedIndex(CurrentOption + 1, true);
	}
}

void UPGOptionSwitcherWidget::UpdateDisplay()
{
	if (OptionText && Options.IsValidIndex(CurrentOption))
	{
		OptionText->SetText(FText::FromString(Options[CurrentOption]));
	}
}

void UPGOptionSwitcherWidget::UpdateButtonStates()
{
	const int32 EffectiveMax = (MaxSelectableIndex >= 0) ? MaxSelectableIndex : Options.Num() - 1;

	if (PrevOptionButton)
	{
		PrevOptionButton->SetIsEnabled(CurrentOption > 0);
	}

	if (NextOptionButton)
	{
		NextOptionButton->SetIsEnabled(CurrentOption < EffectiveMax);
	}
}

