// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PGItemActor.h"
#include "Abilities/GameplayAbility.h"

// Sets default values
APGItemActor::APGItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	//if (ItemData->ItemMesh)
	{
	//	StaticMesh->SetStaticMesh(ItemData->ItemMesh);
	}
}

TSubclassOf<UGameplayAbility> APGItemActor::GetAbilityToInteract() const
{
	return InteractAbility;
}

UPGItemData* APGItemActor::GetItemData()
{
	if (bOwned)
	{
		return nullptr;
	}
	bOwned = true;

	return ItemData;
}
