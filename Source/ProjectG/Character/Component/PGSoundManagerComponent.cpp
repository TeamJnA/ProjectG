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

void UPGSoundManagerComponent::TriggerSoundForSelf(USoundBase* InSound, uint8 SoundVolumeLevel)
{
	SoundManager->PlaySoundForSelf(InSound, SoundVolumeLevel);
}

void UPGSoundManagerComponent::TriggerSoundForAllPlayers_Implementation(USoundBase* InSound, FVector SoundLocation, uint8 SoundPowerLevel)
{
	SoundManager->PlaySoundForAllPlayers(InSound, SoundLocation, SoundPowerLevel);
}

void UPGSoundManagerComponent::TriggerSoundWithNoise_Implementation(USoundBase* InSound, FVector SoundLocation, uint8 SoundPowerLevel, bool bIntensedSound)
{
	SoundManager->PlaySoundWithNoise(InSound, SoundLocation, SoundPowerLevel, bIntensedSound);
}

void UPGSoundManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPGSoundManagerComponent, SoundManager);
}
