// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGSessionStatusWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Throbber.h"

void UPGSessionStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UPGSessionStatusWidget::OnCloseButtonClicked);
	}
}

void UPGSessionStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();
}

FReply UPGSessionStatusWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bClosable && InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnCloseButtonClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPGSessionStatusWidget::SetStatusMessage(const FText& Message, bool bShowCloseButton)
{
	bClosable = bShowCloseButton;

	if (StatusText)
	{
		StatusText->SetText(Message);
	}

	if (CloseButton)
	{
		CloseButton->SetVisibility(bShowCloseButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (Throbber)
	{
		Throbber->SetVisibility(bShowCloseButton ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UPGSessionStatusWidget::OnCloseButtonClicked()
{
	if (UUserWidget* ParentWidget = ReturnFocusWidget.Get())
	{
		ParentWidget->SetKeyboardFocus();
	}
	RemoveFromParent();
}