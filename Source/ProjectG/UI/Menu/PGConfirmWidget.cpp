// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGConfirmWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UPGConfirmWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (YesButton)
	{
		YesButton->OnClicked.AddDynamic(this, &UPGConfirmWidget::OnYesButtonClicked);
	}

	if (NoButton)
	{
		NoButton->OnClicked.AddDynamic(this, &UPGConfirmWidget::OnNoButtonClicked);
	}
}

void UPGConfirmWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();
}

FReply UPGConfirmWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnNoButtonClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPGConfirmWidget::SetConfirmText(const FText& Message)
{
	if (ConfirmText)
	{
		ConfirmText->SetText(Message);
	}
}

void UPGConfirmWidget::OnYesButtonClicked()
{
	OnConfirmClicked.Broadcast();
	RemoveFromParent();
}

void UPGConfirmWidget::OnNoButtonClicked()
{
	if (UUserWidget* ParentWidget = ReturnFocusWidget.Get())
	{
		ParentWidget->SetKeyboardFocus();
	}
	RemoveFromParent();
}