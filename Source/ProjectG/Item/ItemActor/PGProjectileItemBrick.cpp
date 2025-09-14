// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemBrick.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "PGLogChannels.h"


APGProjectileItemBrick::APGProjectileItemBrick()
{
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	CollisionComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UPGItemData> ItemDataRef(TEXT("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick"));
	if (ItemDataRef.Object)
	{
		UPGItemData* ItemData = ItemDataRef.Object;

		ItemDataPath = ItemData;
		LoadedItemData = ItemData;
	}

	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemBrick::OnHit);

	StaticMesh->BodyInstance.bLockXRotation = true;
	StaticMesh->BodyInstance.bLockYRotation = true;
	StaticMesh->BodyInstance.bLockZRotation = true;

	InitialSpeed = 1000.0f;

	bAlreadyHit = false;
}

void APGProjectileItemBrick::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	bAlreadyHit = true;

	// Brick hits once per actor.
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	HitActors.Add(OtherActor);

	// Play sound at hit location.
	if (HasAuthority())
	{
		PlaySound(Hit.Location);
	}

	// Check if the impact normal's Z component indicates a floor (upward-facing surface).
	// If true, disable physics and set the object as an item to pick and re-use.
	ECollisionChannel ObjectType = OtherComponent->GetCollisionObjectType();
	
	if ((ObjectType == ECC_WorldDynamic || ObjectType == ECC_WorldStatic) && Hit.ImpactNormal.Z > 0.6)
	{
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetCollisionProfileName(TEXT("Item"));
		CollisionComponent->SetCollisionProfileName(TEXT("Item"));
	}

	UE_LOG(LogItem, Log, TEXT("Brick hit with actor %s"), *OtherActor->GetName());
}

void APGProjectileItemBrick::PlaySound_Implementation(const FVector& HitLocation)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			if (bAlreadyHit)
			{
				// TODO :: Apply ItemHitSound ITEM_Brick -> ITEM_Brick_Hit
				SoundManager->PlaySoundWithNoise(ItemHitSound, HitLocation, false);
			}
			else
			{
				SoundManager->PlaySoundWithNoise(ItemHitSound, HitLocation, false);
			}
		}
	}
}