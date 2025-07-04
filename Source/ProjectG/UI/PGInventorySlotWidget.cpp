// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGInventorySlotWidget.h"
#include "PGInventorySlotWidget.h"
#include "Item/PGItemData.h"

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
