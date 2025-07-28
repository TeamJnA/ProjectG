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
	bAlwaysRelevant = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	InteractAbility = UGA_Interact_Item::StaticClass();
}

TSubclassOf<UGameplayAbility> APGItemActor::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGItemActor::HighlightOn() const
{
	StaticMesh->SetRenderCustomDepth(true);
}

void APGItemActor::HighlightOff() const
{
	StaticMesh->SetRenderCustomDepth(false);
}

void APGItemActor::InitWithData(UPGItemData* _ItemData)
{
	ItemDataPtr = _ItemData;

	StaticMesh->SetStaticMesh(_ItemData->ItemMesh);

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[SERVER] ItemActor::InitWithData: %s, %s, ItemData: %s"), *GetName(), *GetActorLocation().ToString(), _ItemData ? *_ItemData->GetName() : TEXT("nullptr"));
	}
}

void APGItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGItemActor, ItemDataPtr);
}

void APGItemActor::OnRep_ItemData()
{
	UPGItemData* LoadedItemData = ItemDataPtr.LoadSynchronous();
	if (LoadedItemData)
	{
		StaticMesh->SetStaticMesh(LoadedItemData->ItemMesh);

		if (!HasAuthority())
		{
			UE_LOG(LogTemp, Log, TEXT("[CLIENT] ItemActor::OnRep_ItemData: %s, %s, ItemData: %s"), *GetName(), *GetActorLocation().ToString(), *LoadedItemData->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[CLIENT] ItemActor::OnRep_ItemData %s, ItemData: nullptr"), *GetName());
	}
}

UPGItemData* APGItemActor::GetItemData()
{
	return ItemDataPtr.Get();
}

void APGItemActor::DropItemSpawned()
{
	UE_LOG(LogTemp, Log, TEXT("Set spawned drop item coliision and physics."));
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->OnComponentHit.AddDynamic(this, &APGItemActor::StopItemOnGroundHit);
}

void APGItemActor::StopItemOnGroundHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	ECollisionChannel ObjectType = OtherComponent->GetCollisionObjectType();

	if ((ObjectType == ECC_WorldDynamic || ObjectType == ECC_WorldStatic) && Hit.ImpactNormal.Z > 0.6)
	{
		UE_LOG(LogTemp, Log, TEXT("Drop item hit on ground."));
		StaticMesh->SetSimulatePhysics(false);
	}
}
