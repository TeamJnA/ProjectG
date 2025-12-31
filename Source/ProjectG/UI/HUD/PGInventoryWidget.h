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
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY()
	TArray<TObjectPtr<UPGInventorySlotWidget>> InventorySlots;

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

	TWeakObjectPtr<UPGInventoryComponent> InventoryRef;

	UFUNCTION()
	void HandleOnInventoryUpdate(const TArray<FInventoryItem>& InventoryItems);

	UFUNCTION()
	void HandleOnCurrentSlotIndexChanged(int32 NewIndex);
};
