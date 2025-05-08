// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameInstance.h"

void UPGGameInstance::Init()
{
	Super::Init();

	// Consumable
	ItemDataMap.Add("Brick", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick")));

	// Escape
	ItemDataMap.Add("AdminDevice", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_AdminDevice.DA_Escape_AdminDevice")));
	ItemDataMap.Add("EnergyCore", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_EnergyCore.DA_Escape_EnergyCore")));
	ItemDataMap.Add("RootCalculator", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_RootCalculator.DA_Escape_RootCalculator")));

}

int32 UPGGameInstance::GetMaxInventorySize() const
{
	return MaxInventorySize;
}

UPGItemData* UPGGameInstance::GetItemDataByKey(FName Key)
{
	if (TSoftObjectPtr<UPGItemData>* ptr = ItemDataMap.Find(Key))
	{
		return ptr->LoadSynchronous();
	}
	return nullptr;
}
