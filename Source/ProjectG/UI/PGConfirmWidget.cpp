// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGConfirmWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UPGConfirmWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (YesButton)
	{
		YesButton->OnClicked.AddDynamic(this, &UPGConfirmWidget::OnYesButtonClicked);
	}

	if (NoButton)
	{
		NoButton->OnClicked.AddDynamic(this, &UPGConfirmWidget::OnNoButtonClicked);
	}
}

void UPGConfirmWidget::OnYesButtonClicked()
{
	if (OwningPC.IsValid())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), OwningPC.Get(), EQuitPreference::Quit, true);
	}
}

void UPGConfirmWidget::OnNoButtonClicked()
{
	RemoveFromParent();
}

void UPGConfirmWidget::SetOwningPlayerController(APlayerController* PC)
{
	OwningPC = PC;
}
