// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGInventorySlotWidget.generated.h"

class UPGItemData;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateSlot(UPGItemData* ItemData);

protected:

	UPROPERTY(BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> ItemImage;
};
