// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGInventoryWidget.h"
#include "UI/HUD/PGInventorySlotWidget.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGInventoryComponent.h"

void UPGInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// add inventory slots to InventorySlots array
	InventorySlots.Add(InventorySlot0);
	InventorySlots.Add(InventorySlot1);
	InventorySlots.Add(InventorySlot2);
	InventorySlots.Add(InventorySlot3);
	InventorySlots.Add(InventorySlot4);

	InventorySlot0->HighlightSlot();
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

	InventoryRef = PlayerCharacter->GetInventoryComponent();
	if (InventoryRef)
	{
		InventoryRef->OnInventoryItemUpdate.AddDynamic(this, &UPGInventoryWidget::HandleOnInventoryUpdate);
		InventoryRef->OnCurrentSlotIndexChanged.AddDynamic(this, &UPGInventoryWidget::HandleOnCurrentSlotIndexChanged);

		UE_LOG(LogTemp, Log, TEXT("UPGInventoryWidget::BindInventorySlots: Inventory component delegate binding complete"));
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
	//InventorySlot0->UpdateSlot(InventoryItems[0].ItemData);
	//InventorySlot1->UpdateSlot(InventoryItems[1].ItemData);
	//InventorySlot2->UpdateSlot(InventoryItems[2].ItemData);
	//InventorySlot3->UpdateSlot(InventoryItems[3].ItemData);
	//InventorySlot4->UpdateSlot(InventoryItems[4].ItemData);

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
