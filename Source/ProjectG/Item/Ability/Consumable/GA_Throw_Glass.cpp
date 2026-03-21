// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/Consumable/GA_Throw_Glass.h"
#include "Item/ItemActor/PGProjectileItemGlass.h"
#include "GameFramework/Character.h"
#include "PGLogChannels.h"

UGA_Throw_Glass::UGA_Throw_Glass()
{
	static ConstructorHelpers::FClassFinder<APGProjectileItemGlass> ProjectileItemRef(TEXT("/Game/ProjectG/Items/Consumable/GlassBottle/BP_PGProjectileGlassBottle.BP_PGProjectileGlassBottle_C"));
	if (ProjectileItemRef.Class)
	{
		ProjectileItem = ProjectileItemRef.Class;
	}

	InitialSpeed = 1000.0f;
}

void UGA_Throw_Glass::SpawnProjectileActor()
{
    if (!ProjectileItem)
    {
        UE_LOG(LogPGAbility, Warning, TEXT("Cannot found spawn item in %s"), *GetName());
        return;
    }
    UE_LOG(LogPGAbility, Log, TEXT("SpawnActor in %s"), *GetName());

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    ACharacter* PGPC = Cast<ACharacter>(AvatarActor);
    if (!PGPC) {
        UE_LOG(LogPGAbility, Warning, TEXT("Cannot found avatar actor in %s"), *GetName());
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

    const FVector ForwardSpawnOffset = PGPC->GetActorForwardVector() * (60 + ForwardMovementAmount * 20);

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
        SpawnActorRotation.Roll = -60.0f;
        APGProjectileItemGlass* Projectile = World->SpawnActor<APGProjectileItemGlass>(ProjectileItem, ThrowStartLocation, SpawnActorRotation, SpawnParams);

        if (Projectile)
        {
            Projectile->ThrowInDirection(ThrowStartRotation.Vector(), InitialSpeed);
        }
    }
}
