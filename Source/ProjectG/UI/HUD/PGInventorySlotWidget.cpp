// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGInventorySlotWidget.h"
#include "PGInventorySlotWidget.h"
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
		ItemName = ItemData->ItemName;
		ItemImage = ItemData->ItemTexture2D;
	}
	else
	{
		ItemName = FText::GetEmpty();
		ItemImage = nullptr;
	}	
}

void UPGInventorySlotWidget::HighlightSlot()
{
	if (HighlightBorder)
	{
		HighlightBorder->SetBrushColor(HighlightColor);
	}
}

void UPGInventorySlotWidget::UnhighlightSlot()
{
	if (HighlightBorder)
	{
		HighlightBorder->SetBrushColor(DefaultColor);
	}
}
