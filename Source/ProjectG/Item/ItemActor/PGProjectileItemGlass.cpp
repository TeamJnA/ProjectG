// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemGlass.h"
#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "GameFramework/GameModeBase.h"
#include "NiagaraComponent.h"
#include "PGLogChannels.h"

APGProjectileItemGlass::APGProjectileItemGlass()
{
	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemGlass::OnHit);

	bAlreadyHit = false;

	GlassBreakFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GlassBreakFX"));
	GlassBreakFX->SetupAttachment(StaticMesh);

	GlassBreakFX->bAutoActivate = false;
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

	UE_LOG(LogItem, Log, TEXT("GlassBottle hit with actor %s"), *OtherActor->GetName());

	bAlreadyHit = true;

	// TODO 멀티 환경 테스트
	if (StaticMesh)
	{
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMesh->SetHiddenInGame(true);
	}

	if (GlassBreakFX)
	{
		GlassBreakFX->SetWorldRotation(Hit.ImpactNormal.Rotation());
		GlassBreakFX->Activate(true);
	}

	//TODO [Play sound]
	// PlaySound(Hit.Location); ItemHitSound
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
