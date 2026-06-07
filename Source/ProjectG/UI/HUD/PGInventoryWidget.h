// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGInventoryWidget.generated.h"

class APGPlayerCharacter;
class UPGInventoryComponent;
class UPGInventorySlotWidget;
class UVerticalBox;
class UWidget;
class UPGItemActionGuideWidget;
class UPGItemData;
class UTextBlock;
class UPGGameUserSettings;
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
	void SetHelperGuideAvailable(bool bAvailable);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandlePushToTalkModeChanged(bool bIsPushToTalk);

	UFUNCTION()
	void HandleOnInventoryUpdate(const TArray<FInventoryItem>& InventoryItems);

	UFUNCTION()
	void HandleOnCurrentSlotIndexChanged(int32 NewIndex);

	void UpdateCurrentItemName(const TArray<FInventoryItem>& InventoryItems, int32 CurrentIndex);

	void RebuildItemActionGuide(UPGItemData* CurrentItemData);

	void RefreshKeyGuides();

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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> VoiceKeyGuide;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> CameraKeyGuide;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> HelperKeyGuide;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ItemActionGuideBox;

	UPROPERTY(EditDefaultsOnly, Category = "ActionGuide")
	TSubclassOf<UPGItemActionGuideWidget> ActionGuideEntryClass;

	TWeakObjectPtr<UPGInventoryComponent> InventoryRef;
	TWeakObjectPtr<UPGGameUserSettings> CachedSettings;
	TWeakObjectPtr<UPGItemData> LastGuideItemData;

	int32 CurrentSlotIndex = 0;

	bool bHelperGuideAvailable = false;
};
