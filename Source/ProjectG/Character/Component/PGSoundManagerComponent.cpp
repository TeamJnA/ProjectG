// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGSoundManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/PGSoundManager.h"

// Sets default values for this component's properties
UPGSoundManagerComponent::UPGSoundManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	// PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UPGSoundManagerComponent::SetSoundManager(APGSoundManager* InSoundManager)
{
	SoundManager = InSoundManager;
}

void UPGSoundManagerComponent::TriggerSoundForSelf(FName InSoundName, uint8 SoundVolumeLevel)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *InSoundName.ToString());
		return;
	}
	SoundManager->PlaySoundForSelf(InSoundName, SoundVolumeLevel);
}

void UPGSoundManagerComponent::TriggerSoundForAllPlayers_Implementation(FName SoundName, FVector SoundLocation, uint8 SoundPowerLevel)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *SoundName.ToString());
		return;
	}
	SoundManager->PlaySoundForAllPlayers(SoundName, SoundLocation, SoundPowerLevel);
}

void UPGSoundManagerComponent::TriggerSoundWithNoise_Implementation(FName SoundName, FVector SoundLocation, uint8 SoundPowerLevel, bool bIntensedSound)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *SoundName.ToString());
		return;
	}
	SoundManager->PlaySoundWithNoise(SoundName, SoundLocation, SoundPowerLevel, bIntensedSound);
}

void UPGSoundManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPGSoundManagerComponent, SoundManager);
}
