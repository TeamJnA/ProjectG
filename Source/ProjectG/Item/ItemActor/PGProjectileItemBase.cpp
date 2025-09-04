// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/ArrowComponent.h"


// Sets default values
APGProjectileItemBase::APGProjectileItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(RootComponent);

    /*
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 700.f;
    ProjectileMovementComponent->MaxSpeed = 700.f;
    ProjectileMovementComponent->bShouldBounce = true;
    */

    StaticMesh->SetAllUseCCD(true);

    InitialSpeed = 700.0f;
}

void APGProjectileItemBase::OnHit(UPrimitiveComponent* HitComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComponent, 
    FVector NormalImpulse, 
    const FHitResult& Hit)
{
}

void APGProjectileItemBase::PlaySound_Implementation(const FVector& HitLocation)
{
}
