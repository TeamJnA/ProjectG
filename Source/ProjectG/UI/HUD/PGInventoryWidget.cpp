// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGInventoryWidget.h"
#include "UI/HUD/PGInventorySlotWidget.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGInventoryComponent.h"

void UPGInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// add inventory slots to InventorySlots array
	InventorySlots.Add(InventorySlot0);
	InventorySlots.Add(InventorySlot1);
	InventorySlots.Add(InventorySlot2);
	InventorySlots.Add(InventorySlot3);
	InventorySlots.Add(InventorySlot4);
}

void UPGInventoryWidget::NativeConstruct()
{
	InventorySlot0->HighlightSlot();
}

void UPGInventoryWidget::NativeDestruct()
{
	if (UPGInventoryComponent* Inventory = InventoryRef.Get())
	{
		Inventory->OnInventoryItemUpdate.RemoveAll(this);
		Inventory->OnCurrentSlotIndexChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

/*
* 플레이어 InventoryComponent 바인드
*/
void UPGInventoryWidget::BindInventorySlots(APGPlayerCharacter* PlayerCharacter)
{	
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGInventoryWidget::BindInventorySlots: InPlayerCharacter is NULL!"));
		return;
	}

	if (UPGInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent())
	{
		InventoryRef = Inventory;
		
		Inventory->OnInventoryItemUpdate.RemoveAll(this);
		Inventory->OnInventoryItemUpdate.AddDynamic(this, &UPGInventoryWidget::HandleOnInventoryUpdate);

		Inventory->OnCurrentSlotIndexChanged.RemoveAll(this);
		Inventory->OnCurrentSlotIndexChanged.AddDynamic(this, &UPGInventoryWidget::HandleOnCurrentSlotIndexChanged);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UPGInventoryWidget::BindInventorySlots: Failed to get inventory component"));
	}
}

/*
* When inventory updated, update each inventory slots by delegate
* if item added -> update slot with added item's data
* if item deleted -> update slot to empty
*/
void UPGInventoryWidget::HandleOnInventoryUpdate(const TArray<FInventoryItem>& InventoryItems)
{
	UE_LOG(LogTemp, Log, TEXT("UPGInventoryWidget::HandleOnInventoryUpdate: Update Inventory slot"));

	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventoryItems.IsValidIndex(i) && InventorySlots.IsValidIndex(i))
		{
			InventorySlots[i]->UpdateSlot(InventoryItems[i].ItemData);
		}
	}
}

/*
* highlight inventory slot which local player is selecting
* unhighlight other slots
*/
void UPGInventoryWidget::HandleOnCurrentSlotIndexChanged(int32 NewIndex)
{
	for (int32 i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots.IsValidIndex(i))
		{
			if (i == NewIndex)
			{
				InventorySlots[i]->HighlightSlot();
			}
			else
			{
				InventorySlots[i]->UnhighlightSlot();
			}
		}
	}
}
