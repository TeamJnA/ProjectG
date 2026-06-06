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
	SetIsReplicatedByDefault(true);
}

void UPGSoundManagerComponent::SetSoundManager(APGSoundManager* InSoundManager)
{
	SoundManager = InSoundManager;
	UE_LOG(LogTemp, Log, TEXT("SetSoundManager Completely"));
}

void UPGSoundManagerComponent::TriggerSoundForSelf(FName InSoundName)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *InSoundName.ToString());
		return;
	}
	SoundManager->PlaySoundForSelf(InSoundName);
}

void UPGSoundManagerComponent::TriggerSoundLocally(FName InSoundName, FVector SoundLocation)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *InSoundName.ToString());
		return;
	}
	SoundManager->PlaySoundLocally(InSoundName, SoundLocation);
}

void UPGSoundManagerComponent::TriggerSoundForAllPlayers_Implementation(FName SoundName, FVector SoundLocation)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *SoundName.ToString());
		return;
	}
	SoundManager->PlaySoundForAllPlayers(SoundName, SoundLocation);
}

void UPGSoundManagerComponent::TriggerSoundWithNoise_Implementation(FName SoundName, FVector SoundLocation, bool bIntensedSound)
{
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager in SoundManagerComponent. Called by [%s]"), *SoundName.ToString());
		return;
	}
	SoundManager->PlaySoundWithNoise(SoundName, SoundLocation, bIntensedSound, GetOwner());
}

void UPGSoundManagerComponent::Client_ReportSelfNoise_Implementation(uint8 SoundLevel)
{
	const float Existing = GetCurrentActionNoiseLevel();
	CurrentActionNoiseLevel = FMath::Max(static_cast<float>(SoundLevel), Existing);
	if (const UWorld* World = GetWorld())
	{
		ActionNoiseReportedTime = World->GetTimeSeconds();
	}
}

float UPGSoundManagerComponent::GetCurrentActionNoiseLevel() const
{
	const UWorld* World = GetWorld();
	if (!World || ActionNoiseDecayDuration <= 0.0f)
	{
		return 0.0f;
	}

	const float Elapsed = World->GetTimeSeconds() - ActionNoiseReportedTime;
	if (Elapsed <= ActionNoiseHoldDuration)
	{
		return CurrentActionNoiseLevel;
	}

	const float DecayElapsed = Elapsed - ActionNoiseHoldDuration;
	if (DecayElapsed >= ActionNoiseDecayDuration)
	{
		return 0.0f;
	}
	return CurrentActionNoiseLevel * (1.0f - Elapsed / ActionNoiseDecayDuration);
}

void UPGSoundManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPGSoundManagerComponent, SoundManager);
}
