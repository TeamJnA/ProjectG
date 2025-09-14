// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Consumable/GA_ThrowBrick.h"
#include "GA_ThrowBrick.h"
#include "Item/ItemActor/PGProjectileItemBrick.h"
#include "Character/PGPlayerCharacter.h"
#include "PGLogChannels.h"


UGA_ThrowBrick::UGA_ThrowBrick()
{
   static ConstructorHelpers::FClassFinder<APGProjectileItemBrick> ProjectileItemRef(TEXT("/Game/ProjectG/Items/Consumable/Brick/BP_PGProjectileItemBrick.BP_PGProjectileItemBrick_C"));
   if (ProjectileItemRef.Class)
   {
       ProjectileItem = ProjectileItemRef.Class;
   }
}

void UGA_ThrowBrick::SpawnProjectileActor()
{
    if (!ProjectileItem)
    {
        UE_LOG(LogAbility, Warning, TEXT("Cannot found spawn item in %s"), *GetName());
        return;
    }
    UE_LOG(LogAbility, Log, TEXT("SpawnActor in %s"), *GetName());
    
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
    if (!PGPC) {
        UE_LOG(LogAbility, Warning, TEXT("Cannot found avatar actor in %s"), *GetName());
        return;
    }
    
    // Start location is little right of character eye.
    FVector ThrowStartLocation;
    FRotator ThrowStartRotation;

    PGPC->GetActorEyesViewPoint(ThrowStartLocation, ThrowStartRotation);
    
    // Check if the character is moving forward; 
    // If true, spawn the object slightly farther ahead.
    float ForwardMovementAmount = FVector::DotProduct(PGPC->GetVelocity().GetSafeNormal(), PGPC->GetActorForwardVector());
    if (ForwardMovementAmount < 0)
    {
        ForwardMovementAmount = 0;
    }

    const FVector ForwardSpawnOffset = PGPC->GetActorForwardVector() * (40 + ForwardMovementAmount * 20);

    ThrowStartLocation += ForwardSpawnOffset + (PGPC->GetActorRightVector() * 25);
    ThrowStartRotation.Pitch += 10.0f;

    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwningActorFromActorInfo();
        SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());

        FRotator SpawnActorRotation;
        SpawnActorRotation = FRotator::ZeroRotator;
        SpawnActorRotation.Yaw += 90.0f;
        APGProjectileItemBrick* Projectile = World->SpawnActor<APGProjectileItemBrick>(ProjectileItem, ThrowStartLocation, SpawnActorRotation, SpawnParams);
        
        if (Projectile)
        {
            Projectile->ThrowInDirection(ThrowStartRotation.Vector());
        }
    }
}