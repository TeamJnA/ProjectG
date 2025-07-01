// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemBrick.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "GameFramework/GameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"


APGProjectileItemBrick::APGProjectileItemBrick()
{
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	CollisionComponent->SetupAttachment(RootComponent);

	//ItemData
	static ConstructorHelpers::FObjectFinder<UPGItemData> ItemDataRef(TEXT("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick"));
	if (ItemDataRef.Object)
	{
		ItemData = ItemDataRef.Object;
	}

	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemBrick::OnHit);

	StaticMesh->BodyInstance.bLockXRotation = true;
	StaticMesh->BodyInstance.bLockYRotation = true;
	StaticMesh->BodyInstance.bLockZRotation = true;

	InitialSpeed = 1000.0f;

	bAlreadyHit = false;
}

void APGProjectileItemBrick::ThrowInDirection(const FVector& ShootDirection)
{
	StaticMesh->SetPhysicsLinearVelocity(ShootDirection * InitialSpeed);
}

void APGProjectileItemBrick::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	bAlreadyHit = true;

	// This process executed once per actor.
	if (HitActors.Contains(OtherActor))
		return;
	HitActors.Add(OtherActor);

	// Play sound at hit location.
	if(bAlreadyHit)
		PlaySound(Hit.Location, 2);
	else
		PlaySound(Hit.Location, 3);

	// Check if the impact normal's Z component indicates a floor (upward-facing surface).
	// If true, disable physics and set the object as an item.
	ECollisionChannel ObjectType = OtherComponent->GetCollisionObjectType();
	
	if ((ObjectType == ECC_WorldDynamic || ObjectType == ECC_WorldStatic) && Hit.ImpactNormal.Z > 0.6)
	{
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetCollisionProfileName(TEXT("Item"));
		CollisionComponent->SetCollisionProfileName(TEXT("Item"));
	}

	UE_LOG(LogTemp, Log, TEXT("Brick hit with actor %s"), *OtherActor->GetName());
}

void APGProjectileItemBrick::PlaySound_Implementation(const FVector& HitLocation, uint8 HitImpact)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager();
		if (SoundManager)
		{
			UE_LOG(LogTemp, Log, TEXT("Call Soundmanager in Brick"));

			SoundManager->PlaySoundWithNoise(HitSound, HitLocation, HitImpact, false);
		}
	}
}