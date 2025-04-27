// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGInventoryComponent.h"
#include "Item/PGItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Game/PGGameInstance.h"

// Sets default values for this component's properties
UPGInventoryComponent::UPGInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = false;

	//Get max Inventory size and set item inventory max size
	UPGGameInstance* PGGameInstance = Cast<UPGGameInstance>
		(UGameplayStatics::GetGameInstance(GetWorld()));
	
	if (PGGameInstance){
		MaxInventorySize = PGGameInstance->GetMaxInventorySize();
		Items.SetNum(MaxInventorySize);
	}
}


// Called when the game starts
void UPGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UPGInventoryComponent::ChangeCurrectItemIndex(int32 NewItemIndex)
{
	CurrentItemIndex = NewItemIndex;
}
