// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMessageEntryWidget.h"

void UPGMessageEntryWidget::SetMessage(FText _Message)
{
	if (!Message.IsEmpty())
	{
		Message = _Message;
	}
	else
	{
		Message = FText::GetEmpty();
	}
}
