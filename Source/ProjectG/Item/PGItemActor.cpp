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

/*
* 
*/
void APGItemActor::HighlightOn() const
{
	StaticMesh->SetRenderCustomDepth(true);
}

/*
* 
*/
void APGItemActor::HighlightOff() const
{
	StaticMesh->SetRenderCustomDepth(false);
}

/*
* Item Actor의 InteractionInfo 반환
* Instant Input
*/
FInteractionInfo APGItemActor::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Instant);
}

/*
* 로드한 아이템 데이터로 생성한 아이템 세팅
*/
void APGItemActor::InitWithData(UPGItemData* InItemData)
{
	if (!HasAuthority())
	{
		return;
	}

	LoadedItemData = InItemData;
	ItemDataPath = InItemData;

	if (InItemData)
	{
		StaticMesh->SetStaticMesh(InItemData->ItemMesh);
	}

	UE_LOG(LogTemp, Log, TEXT("[SERVER] ItemActor::InitWithData: %s, %s, ItemData: %s"), *GetName(), *GetActorLocation().ToString(), InItemData ? *InItemData->GetName() : TEXT("nullptr"));
}

void APGItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGItemActor, ItemDataPath);
}

/*
* 아이템 데이터 경로 레플리케이션 후 클라이언트 호출
*/
void APGItemActor::OnRep_ItemData()
{
	UPGItemData* ItemData = ItemDataPath.LoadSynchronous();
	if (ItemData)
	{
		LoadedItemData = ItemData;
		StaticMesh->SetStaticMesh(ItemData->ItemMesh);
	}
}

UPGItemData* APGItemActor::GetItemData()
{
	return LoadedItemData;
}

void APGItemActor::DropItemSpawned(const FRotator DropRotation)
{
	UE_LOG(LogTemp, Log, TEXT("Set spawned drop item coliision and physics."));
	StaticMesh->SetSimulatePhysics(true);

	const FVector DropDirection = DropRotation.Vector();
	const float ImpulseStrength = 1500.0f;

	StaticMesh->AddImpulse(DropDirection * ImpulseStrength);

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
