// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemBrick.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
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

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APGProjectileItemBrick::OnOverlapBegin);

	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemBrick::OnHit);

	StaticMesh->BodyInstance.bLockXRotation = true;
	StaticMesh->BodyInstance.bLockYRotation = true;
	StaticMesh->BodyInstance.bLockZRotation = true;

	InitialSpeed = 1000.0f;
}

void APGProjectileItemBrick::ThrowInDirection(const FVector& ShootDirection)
{
	StaticMesh->SetPhysicsLinearVelocity(ShootDirection * InitialSpeed);
}

void APGProjectileItemBrick::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor == this)
		return;
	
	if (!HasAuthority())
		return;

	UE_LOG(LogTemp, Log, TEXT("Brick overlap with component"));
	// If otherActor == instigator, return
	// 그 외의 경우에는, 사운드 재생
	// 
	if (OtherActor != GetInstigator())
	{
		UE_LOG(LogTemp, Log, TEXT("Brick overlap with other Actor! %s"), *OtherActor->GetName());
	}
}
void APGProjectileItemBrick::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Log, TEXT("Brick hit with actor"));
}
//Init With Data.....