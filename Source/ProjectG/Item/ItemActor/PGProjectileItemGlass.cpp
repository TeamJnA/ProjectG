// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemGlass.h"
#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "GameFramework/GameModeBase.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h" 
#include "PGLogChannels.h"

APGProjectileItemGlass::APGProjectileItemGlass()
{
	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemGlass::OnHit);

	bAlreadyHit = false;

	GlassBreakFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GlassBreakFX"));
	GlassBreakFX->SetupAttachment(StaticMesh);

	GlassBreakFX->bAutoActivate = false;

	HitRotation = FRotator::ZeroRotator;
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

	HitRotation = Hit.ImpactNormal.Rotation();
	OnRep_IsBroken();

	PlaySound(Hit.Location);

	SetLifeSpan(3.0f);
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

void APGProjectileItemGlass::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGProjectileItemGlass, HitRotation);
}

void APGProjectileItemGlass::OnRep_IsBroken()
{
	if (StaticMesh)
	{
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMesh->SetHiddenInGame(true);
	}

	if (GlassBreakFX)
	{
		GlassBreakFX->SetWorldRotation(HitRotation);
		GlassBreakFX->Activate(true);
	}
}
