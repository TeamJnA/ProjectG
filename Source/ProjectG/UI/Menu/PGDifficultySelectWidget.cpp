// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGDifficultySelectWidget.h"
#include "Components/Button.h"


void UPGDifficultySelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (NormalButton)
	{
		NormalButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnNormalClicked);
	}

	if (HardButton)
	{
		HardButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnHardClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnCancelClicked);
	}
}

void UPGDifficultySelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();
}

FReply UPGDifficultySelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnCancelClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPGDifficultySelectWidget::OnNormalClicked()
{
	OnDifficultySelected.Broadcast(EPGDifficulty::Normal);
	RemoveFromParent();
}

void UPGDifficultySelectWidget::OnHardClicked()
{
	OnDifficultySelected.Broadcast(EPGDifficulty::Hard);
	RemoveFromParent();
}

void UPGDifficultySelectWidget::OnCancelClicked()
{
	if (UUserWidget* Parent = ReturnFocusWidget.Get())
	{
		Parent->SetKeyboardFocus();
	}
	RemoveFromParent();
}
