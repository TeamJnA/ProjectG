// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGItemActionGuideWidget.h"
#include "Components/TextBlock.h"
#include "COmponents/Image.h"


void UPGItemActionGuideWidget::Setup(const FText& InActionText, UTexture2D* InKeyIcon, const FVector2D& InKeySize)
{
	if (ActionText)
	{
		ActionText->SetText(InActionText);
	}

	if (!KeyImage)
	{
		return;
	}

	if (!InKeyIcon)
	{
		KeyImage->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(InKeyIcon);
	Brush.ImageSize = InKeySize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	KeyImage->SetBrush(Brush);
	KeyImage->SetDesiredSizeOverride(InKeySize);
	KeyImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}
