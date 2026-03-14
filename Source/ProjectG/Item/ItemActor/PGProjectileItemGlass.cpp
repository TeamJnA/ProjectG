// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemGlass.h"
#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"

APGProjectileItemGlass::APGProjectileItemGlass()
{
	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemGlass::OnHit);

	bAlreadyHit = false;
}

void APGProjectileItemGlass::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bAlreadyHit)
	{
		return;
	}

	bAlreadyHit = true;

	//TODO [Play sound], [glass break effect]
	// PlaySound(Hit.Location);
}

void APGProjectileItemGlass::PlaySound_Implementation(const FVector& HitLocation)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			SoundManager->PlaySoundWithNoise(ItemHitSound, HitLocation, false);
		}
	}
}
