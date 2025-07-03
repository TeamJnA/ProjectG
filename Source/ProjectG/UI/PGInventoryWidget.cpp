// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGInventoryWidget.h"
#include "UI/PGInventorySlotWidget.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGInventoryComponent.h"

void UPGInventoryWidget::BindInventorySlots(APGPlayerCharacter* PlayerCharacter)
{	
	//APlayerController* Controller = GetOwningPlayer();
	//if (!Controller) return;

	//APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(Controller->GetPawn());
	//if (!PGCharacter) return;

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGInventoryWidget::BindInventorySlots: InPlayerCharacter is NULL!"));
		return;
	}

	InventoryRef = PlayerCharacter->GetInventoryComponent();
	if (InventoryRef)
	{
		/*
		* for debug
		*/
		ACharacter* Char = Cast<ACharacter>(InventoryRef->GetOwner());
		if (Char)
		{
			UE_LOG(LogTemp, Log, TEXT("UPGInventoryWidget::BindInventorySlots: %s | IsLocal : %d | HasAuthority : %d"), *Char->Controller->GetName(), Char->Controller->IsLocalController(), Char->Controller->HasAuthority());
		}
		InventoryRef->OnInventoryItemUpdate.AddDynamic(this, &UPGInventoryWidget::HandleOnInventoryUpdate);
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
	InventorySlot0->UpdateSlot(InventoryItems[0].ItemData);
	InventorySlot1->UpdateSlot(InventoryItems[1].ItemData);
	InventorySlot2->UpdateSlot(InventoryItems[2].ItemData);
	InventorySlot3->UpdateSlot(InventoryItems[3].ItemData);
	InventorySlot4->UpdateSlot(InventoryItems[4].ItemData);
}
