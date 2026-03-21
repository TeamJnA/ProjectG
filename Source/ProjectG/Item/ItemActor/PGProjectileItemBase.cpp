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

    float RandomSpinIntensity = FMath::FRandRange(400.f, 800.f);


    FVector AngularVel = FVector(
        FMath::FRandRange(-1.f, 1.f), // Roll 방향 랜덤성
        1.0f,                         // 주 회전축 (Pitch)
        FMath::FRandRange(-0.5f, 0.5f) // Yaw 방향 랜덤성
    );

    AngularVel.Normalize(); // 방향 벡터 정규화
    StaticMesh->SetPhysicsAngularVelocityInDegrees(AngularVel * RandomSpinIntensity);
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
