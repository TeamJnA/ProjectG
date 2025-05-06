// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGInventoryComponent.h"
#include "Item/PGItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Game/PGGameInstance.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UPGInventoryComponent::UPGInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = false;

	bInventoryFull = false;

	InventoryItemCount = 0;

	SetIsReplicated(true);
}


// Called when the game starts
void UPGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	//Get max Inventory size and set item inventory max size
	UPGGameInstance* PGGameInstance = Cast<UPGGameInstance>
		(UGameplayStatics::GetGameInstance(GetWorld()));

	if (PGGameInstance) {
		MaxInventorySize = PGGameInstance->GetMaxInventorySize();
		Items.SetNum(MaxInventorySize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NotSetINSTNADTK"));
	}
	for (int32 i = 0; i < MaxInventorySize; i++)
		Items[i] = nullptr;
}

void UPGInventoryComponent::ChangeCurrectItemIndex(int32 NewItemIndex)
{
	CurrentItemIndex = NewItemIndex;
	UE_LOG(LogTemp, Log, TEXT("Current Item is %d"), CurrentItemIndex);
	if(Items[CurrentItemIndex])
		UE_LOG(LogTemp, Log, TEXT("Current Item is %s"), *Items[CurrentItemIndex]->ItemName.ToString());
}

void UPGInventoryComponent::AddItemToInventory(UPGItemData* ItemData)
{
	if (bInventoryFull)
		return;
	int32 ItemInputIdx = CurrentItemIndex;

	//If currentidx already has item, get the frontest idx of itemArray.
	if (Items[ItemInputIdx])
	{
		for (int32 i = 0; i < MaxInventorySize; i++)
		{
			if (!Items[i])
			{
				ItemInputIdx = i;
				break;
			}
		}
	}
	Items[ItemInputIdx] = ItemData;
	InventoryItemCount++;
	if (InventoryItemCount == MaxInventorySize)
		bInventoryFull = true;
	UE_LOG(LogTemp, Log, TEXT("Get Item %s in %s 's InventoryComponent"), *Items[ItemInputIdx]->ItemName.ToString(), *GetOwner()->GetName())
}

bool UPGInventoryComponent::IsInventoryFull()
{
	return bInventoryFull;
}

void UPGInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPGInventoryComponent, Items);
}
