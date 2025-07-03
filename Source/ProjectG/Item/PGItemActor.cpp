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

void APGItemActor::HighlightOn() const
{
	StaticMesh->SetRenderCustomDepth(true);
}

void APGItemActor::HighlightOff() const
{
	StaticMesh->SetRenderCustomDepth(false);
}

void APGItemActor::InitWithData_Implementation(UPGItemData* InData)
{
	ItemData = InData;

	StaticMesh->SetStaticMesh(ItemData->ItemMesh);
}

void APGItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGItemActor, ItemData);
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

UPGItemData* APGItemActor::GetItemData()
{
	return ItemData;
}


void APGItemActor::PostNetInit()
{
	Super::PostNetInit();
	if (!HasAuthority()) // 클라이언트에서만
	{
		UE_LOG(LogTemp, Warning, TEXT("CLIENT POSTNETINIT Item: Name=%s, Location=%s, NetGUID=%s"), *GetName(), *GetActorLocation().ToString(), *GetActorGuid().ToString());
	}
}

void APGItemActor::PostNetReceive()
{
	Super::PostNetReceive();
	if (!HasAuthority()) // 클라이언트에서만
	{
		UE_LOG(LogTemp, Warning, TEXT("CLIENT PostNetReceive Item: Name=%s, Location=%s, NetGUID=%s"), *GetName(), *GetActorLocation().ToString(), *GetActorGuid().ToString());
	}
}