// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMessageEntryWidget.h"

void UPGMessageEntryWidget::SetMessage(FText _Message)
{
	//UE_LOG(LogTemp, Warning, TEXT("UPGMessageEntryWidget::SetMessage: Set Message"));
	Message = _Message;
	//if (!Message.IsEmpty())
	//{
	//	Message = _Message;
	//}
	//else
	//{
	//	Message = FText::GetEmpty();
	//}
}
