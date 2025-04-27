// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PGItemActor.h"

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

void APGItemActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AddTag(ItemData->ItemTag);
}