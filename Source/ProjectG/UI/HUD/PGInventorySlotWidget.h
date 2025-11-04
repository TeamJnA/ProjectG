// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGInventorySlotWidget.generated.h"

class UBorder;
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

	void HighlightSlot();
	void UnhighlightSlot();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	FText ItemName;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> ItemImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> HighlightBorder;

	UPROPERTY(EditAnywhere, Category = "UI")
	FLinearColor HighlightColor;

	UPROPERTY(EditAnywhere, Category = "UI")
	FLinearColor DefaultColor = FLinearColor::Transparent;
};
