// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PGItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbility.h"
#include "Interact/Ability/GA_Interact_Item.h"

// Sets default values
APGItemActor::APGItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	InteractAbility = UGA_Interact_Item::StaticClass();
}

TSubclassOf<UGameplayAbility> APGItemActor::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGItemActor::InitWithData(UPGItemData* InData)
{
	ItemData = InData;

	if (HasAuthority())
	{
		OnRep_ItemData();
	}
}

void APGItemActor::OnRep_ItemData()
{
	if (ItemData && StaticMesh)
	{
		StaticMesh->SetStaticMesh(ItemData->ItemMesh);
	}
}

void APGItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGItemActor, ItemData);
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
