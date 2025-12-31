// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGMessageEntryWidget.h"

void UPGMessageEntryWidget::SetMessage(FText InMessage, float Duration)
{
	Message = InMessage;

	GetWorld()->GetTimerManager().ClearTimer(MessageTimerHandle);
	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, this, &UPGMessageEntryWidget::ClearMessage, Duration, false);
	}
}

void UPGMessageEntryWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageTimerHandle);
	}

	Super::NativeDestruct();
}

void UPGMessageEntryWidget::ClearMessage()
{
	Message = FText::GetEmpty();
}
