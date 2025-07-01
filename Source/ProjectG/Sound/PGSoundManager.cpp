// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
APGSoundManager::APGSoundManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...

	// SetReplicates(true);
	bReplicates = true;

	bAlwaysRelevant = true;

	static ConstructorHelpers::FObjectFinder<USoundAttenuation> SoundAttenuationRef(TEXT("/Game/ProjectG/Sound/BaseSoundAttenuation.BaseSoundAttenuation"));
	if (SoundAttenuationRef.Object)
	{
		BaseSoundAttenuation = SoundAttenuationRef.Object;
	}

	bDebugSoundRange = true;
}


// Called when the game starts
void APGSoundManager::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void APGSoundManager::PlaySoundForSelf(USoundBase* SoundAsset, uint8 SoundVolumeLevel)
{
	if (!SoundAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundAsset is Null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Play sound %s in PlaySoundForSelf"), *SoundAsset->GetName());
	UGameplayStatics::PlaySound2D(GetWorld(), SoundAsset, 1);
}

void APGSoundManager::PlaySoundForAllPlayers_Implementation(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundPowerLevel)
{
	if (!SoundAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundAsset is Null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Play sound for all players %s in PlaySoundForAllPlayers"), *SoundAsset->GetName());

	PlaySoundMulticast(SoundAsset, SoundLocation, SoundPowerLevel);
}

void APGSoundManager::PlaySoundWithNoise_Implementation(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundPowerLevel, bool bIntensedSound)
{
	if (!SoundAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundAsset is Null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Start sound multicast %s in PlaySoundWithNoise"), *SoundAsset->GetName());
	PlaySoundMulticast(SoundAsset, SoundLocation, SoundPowerLevel);

	// Make noise for enemy AI chase sound.
	float SoundRange = 200 * SoundPowerLevel * SoundPowerLevel;
	if (bIntensedSound)
		SoundPowerLevel++;

	MakeNoise(SoundPowerLevel, nullptr, SoundLocation, SoundRange);

	// Draw debug sphere of makenoise range
	if (bDebugSoundRange)
	{
		if (bIntensedSound)
		{
			DrawDebugSphere(GetWorld(), SoundLocation, SoundRange, 8, FColor::Red, false, 3.0f, 0U, 3.0f);
		}
		else
		{
			float StrongSoundRange = 200 * (SoundPowerLevel - 1) * (SoundPowerLevel - 1);
			DrawDebugSphere(GetWorld(), SoundLocation, StrongSoundRange, 8, FColor::Red, false, 3.0f, 0U, 3.0f);
			DrawDebugSphere(GetWorld(), SoundLocation, SoundRange, 8, FColor::Yellow, false, 3.0f, 0U, 3.0f);
		}
	}
	
}

void APGSoundManager::PlaySoundMulticast_Implementation(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundPowerLevel)
{
	// Sets the audible range and the radius where sound attenuation starts.
	float AttenuationExtentRange, AttenuationFalloffDistance;
	AttenuationExtentRange = 200 * SoundPowerLevel * SoundPowerLevel;
	AttenuationFalloffDistance = 200 * (SoundPowerLevel - 1) * (SoundPowerLevel - 1);

	// AttenuationShapeExtents.X is the sphere radius of attenuation.
	BaseSoundAttenuation->Attenuation.AttenuationShapeExtents.X = AttenuationExtentRange;
	BaseSoundAttenuation->Attenuation.FalloffDistance = AttenuationFalloffDistance;

	UE_LOG(LogTemp, Log, TEXT("Play sound %s in PlaySoundMulticast"), *SoundAsset->GetName());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundAsset, SoundLocation, 1.0f, 1.0f, 0.0f, BaseSoundAttenuation);
}

