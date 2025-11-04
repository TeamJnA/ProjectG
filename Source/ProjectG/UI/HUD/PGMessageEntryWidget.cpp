// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGMessageEntryWidget.h"

void UPGMessageEntryWidget::SetMessage(FText _Message, float Duration)
{
	Message = _Message;

	GetWorld()->GetTimerManager().ClearTimer(MessageTimerHandle);
	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, this, &UPGMessageEntryWidget::ClearMessage, Duration, false);
	}
}

void UPGMessageEntryWidget::ClearMessage()
{
	Message = FText::GetEmpty();
}
