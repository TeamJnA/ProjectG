// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGInventoryWidget.generated.h"

class APGPlayerCharacter;
class UPGInventoryComponent;
class UPGInventorySlotWidget;
class UPGItemData;
struct FInventoryItem;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void BindInventorySlots(APGPlayerCharacter* PlayerCharacter);

protected:
	TObjectPtr<UPGInventoryComponent> InventoryRef;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> InventorySlot0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> InventorySlot1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> InventorySlot2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> InventorySlot3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGInventorySlotWidget> InventorySlot4;

	UFUNCTION()
	void HandleOnInventoryUpdate(const TArray<FInventoryItem>& InventoryItems);
};
