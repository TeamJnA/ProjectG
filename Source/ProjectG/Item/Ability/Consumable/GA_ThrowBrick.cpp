// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Consumable/GA_ThrowBrick.h"
#include "GA_ThrowBrick.h"
#include "Item/ItemActor/PGProjectileItemBrick.h"
#include "Character/PGPlayerCharacter.h"


UGA_ThrowBrick::UGA_ThrowBrick()
{
   static ConstructorHelpers::FClassFinder<APGProjectileItemBrick> ProjectileItemRef(TEXT("/Game/ProjectG/Items/Consumable/BP_PGProjectileItemBrick.BP_PGProjectileItemBrick_C"));
   if (ProjectileItemRef.Class)
   {
       ProjectileItem = ProjectileItemRef.Class;
   }
   else
   {
       UE_LOG(LogAbility, Warning, TEXT("Cannot found ProjectileItemRef in %s"), *GetName());
   }
}

void UGA_ThrowBrick::SpawnProjectileActor()
{
	UE_LOG(LogAbility, Log, TEXT("SpawnActor in %s"), *GetName());
    if (!ProjectileItem)
    {
        UE_LOG(LogAbility, Warning, TEXT("Cannot found spawn item in %s"), *GetName());
        return;
    }
    
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
    if (!PGPC) {
        UE_LOG(LogAbility, Warning, TEXT("Cannot found avatar actor in %s"), *GetName());
        return;
    }
    
    //Start location is little right of character eye.
    //
    FVector ThrowStartLocation;
    FRotator ThrowStartRotation;

    PGPC->GetActorEyesViewPoint(ThrowStartLocation, ThrowStartRotation);
    ThrowStartLocation = ThrowStartLocation + PGPC->GetActorRightVector() * 25 + PGPC->GetActorForwardVector() * 40;
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
            // Delay the throw to ensure the mesh's physics initialized.
            // ThrowInDirection(SetPhysicsLinearVelocity) could be ignored if the physics system isn't ready yet.
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [Projectile, ThrowStartRotation]()
                {
                    if (Projectile && Projectile->IsValidLowLevel())
                    {
                        Projectile->ThrowInDirection(ThrowStartRotation.Vector());
                    }

                }, 0.01f, false);
        }
    }

}