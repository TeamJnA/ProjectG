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
}


// Called when the game starts
void APGSoundManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void APGSoundManager::PlaySoundForSelf(USoundBase* SoundAsset, uint8 SoundVolumeLevel)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaySoundForSelf failed: World is null."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Play sound %s in PlaySoundForSelf"), *SoundAsset->GetName());
	UGameplayStatics::PlaySound2D(GetWorld(), SoundAsset, 1);
}

void APGSoundManager::PlaySoundWithNoise_Implementation(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundVolumeLevel, uint8 SoundRangeLevel)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaySoundWithNoise failed: World is null."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Start to multicast sound %s in PlaySoundWithNoise"), *SoundAsset->GetName());
	PlaySoundMulticast(SoundAsset, SoundLocation, SoundVolumeLevel, SoundRangeLevel);
}

//void APGSoundManager::PlaySoundClient(USoundBase* SoundAsset, uint8 SoundVolumeLevel)

void APGSoundManager::PlaySoundMulticast_Implementation(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundVolumeLevel, uint8 SoundRangeLevel)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaySoundMulticast failed: World is null."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Play sound %s in PlaySoundWithNoise, PlaySoundMulticast"), *SoundAsset->GetName());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundAsset, SoundLocation, SoundVolumeLevel);
}

