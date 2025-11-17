// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/ArrowComponent.h"

APGProjectileItemBase::APGProjectileItemBase()
{
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->SetupAttachment(RootComponent);

    StaticMesh->SetAllUseCCD(true);
}

void APGProjectileItemBase::ThrowInDirection(const FVector& ShootDirection, const float InitialSpeed)
{
    StaticMesh->SetPhysicsLinearVelocity(ShootDirection * InitialSpeed);
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
