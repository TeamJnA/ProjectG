// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGSessionStatusWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Throbber.h"

void UPGSessionStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UPGSessionStatusWidget::OnCloseButtonClicked);
	}
}

void UPGSessionStatusWidget::SetStatusMessage(const FText& Message, bool bShowCloseButton)
{
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
	RemoveFromParent();
}