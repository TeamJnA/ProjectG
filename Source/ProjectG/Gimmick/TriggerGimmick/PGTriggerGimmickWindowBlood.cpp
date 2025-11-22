// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickWindowBlood.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"

APGTriggerGimmickWindowBlood::APGTriggerGimmickWindowBlood()
{
	ActivationChance = 0.5f;
	bIsOneShotEvent = true;

	if (StaticMesh)
	{
		StaticMesh->SetCollisionProfileName(TEXT("BlockAll"));
		StaticMesh->SetGenerateOverlapEvents(false);
	}
}

void APGTriggerGimmickWindowBlood::BeginPlay()
{
	Super::BeginPlay();

	if (StaticMesh && StaticMesh->GetMaterial(0))
	{
		DynamicWindowMaterial = StaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void APGTriggerGimmickWindowBlood::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnTriggerOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (bHasBeenTriggered)
	{
		Multicast_PlayWindowEffect(GetActorLocation());
	}
}

void APGTriggerGimmickWindowBlood::Multicast_PlayWindowEffect_Implementation(const FVector& HitLocation)
{
	if (DynamicWindowMaterial)
	{
		DynamicWindowMaterial->SetScalarParameterValue(FName("BloodIntensity"), 1.0f);
	}

	//if (ISoundManagerInterface* SMInterface = Cast<ISoundManagerInterface>(UGameplayStatics::GetGameMode(this)))
	//{
	//	if (APGSoundManager* SoundManager = SMInterface->GetSoundManager())
	//	{
	//		SoundManager->PlaySoundForAllPlayers(WindowHitSoundName, HitLocation);
	//	}
	//}
}
