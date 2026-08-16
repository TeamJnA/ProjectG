// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGInventorySlotWidget.h"
#include "Item/PGItemData.h"
#include "Components/Border.h"

/*
* Update inventory slot widget
* if ItemData is not valid -> set empty
* this means no item in the slot
*/
void UPGInventorySlotWidget::UpdateSlot(UPGItemData* ItemData)
{
	if (ItemData)
	{
		ItemImage = ItemData->ItemTexture2D;
	}
	else
	{
		ItemImage = nullptr;
	}	
}

void UPGInventorySlotWidget::HighlightSlot()
{
	// Set inner highlight
	if (HighlightBorder)
	{
		HighlightBorder->SetBrushColor(HighlightColor);
	}

	// Change Outline color
	if (!OutlineBorderMID)
	{
		if (OutlineBorder)
		{
			OutlineBorderMID = OutlineBorder->GetDynamicMaterial();
		}
	}

	if (!OutlineBorderMID)
	{
		return;
	}

	OutlineBorderMID->SetVectorParameterValue(OutlineColorParam, FLinearColor(1.0f, 1.0f, 1.0f));
}

void UPGInventorySlotWidget::UnhighlightSlot()
{
	// Set inner highlight
	if (HighlightBorder)
	{
		HighlightBorder->SetBrushColor(DefaultColor);
	}

	// Change Outline color
	if (!OutlineBorderMID)
	{
		if (OutlineBorder)
		{
			OutlineBorderMID = OutlineBorder->GetDynamicMaterial();
		}
	}

	if (!OutlineBorderMID)
	{
		return;
	}

	OutlineBorderMID->SetVectorParameterValue(OutlineColorParam, FLinearColor(0.01f, 0.017f, 0.023f));
}
