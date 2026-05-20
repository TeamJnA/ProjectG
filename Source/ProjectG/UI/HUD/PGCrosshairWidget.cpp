// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGCrosshairWidget.h"
#include "Components/Image.h"


void UPGCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetCrosshairVisible(true);
}

void UPGCrosshairWidget::SetCrosshairVisible(bool bVisible)
{
	if (CrosshairImage)
	{
		CrosshairImage->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}
