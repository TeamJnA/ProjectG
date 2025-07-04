// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

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

	bDebugSoundRange = true;

	static ConstructorHelpers::FObjectFinder<USoundAttenuation> SoundAttenuationRef(TEXT("/Game/ProjectG/Sound/BaseSoundAttenuation.BaseSoundAttenuation"));
	if (SoundAttenuationRef.Object)
	{
		BaseSoundAttenuation = SoundAttenuationRef.Object;
	}

	// Load SoundDatas from DataTable 
	static ConstructorHelpers::FObjectFinder<UDataTable> SoundDataTableRef(TEXT("/Game/ProjectG/Sound/PGSoundDataTable.PGSoundDataTable"));
	if (SoundDataTableRef.Object)
	{
		UDataTable* SoundDataTable = SoundDataTableRef.Object;

		// Get a map of all the rows in the DataTable
		// The map key is the row name, and the value is a pointer to the row data (uint8*)
		const TMap<FName, uint8*>& DataTableRows = SoundDataTable->GetRowMap();

		// Loop through each row
		for (const TPair<FName, uint8*>& DataTableRow : DataTableRows)
		{
			FName RowName = DataTableRow.Key;

			// Cast the row data from pointer(uint8*) to our custom struct type
			FPGSoundPlayData* RowData = reinterpret_cast<FPGSoundPlayData*>(DataTableRow.Value);

			if (RowData && RowData->SoundAsset)
			{
				UE_LOG(LogTemp, Log, TEXT("Add SoundData to SoundDataMap : %s"), *RowName.ToString());
				SoundDataMap.Add(RowName, FPGSoundPlayData(RowData->SoundAsset, RowData->SoundStartTime));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid Row or missing SoundAsset for row: %s"), *RowName.ToString());
			}
		}
	}
}


// Called when the game starts
void APGSoundManager::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void APGSoundManager::PlaySoundForSelf(FName SoundName, uint8 SoundVolumeLevel)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundData from %s"), *SoundName.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Play sound %s in PlaySoundForSelf"), *SoundName.ToString());
	UGameplayStatics::PlaySound2D(GetWorld(), SoundData->SoundAsset, 1.0f, 1.0f, SoundData->SoundStartTime);
}

void APGSoundManager::PlaySoundForAllPlayers_Implementation(FName SoundName, FVector SoundLocation, uint8 SoundPowerLevel)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundData from %s"), *SoundName.ToString());
		return;
	}

	// Play sound for all players by execute PlaySoundMulticast.
	UE_LOG(LogTemp, Log, TEXT("Play sound for all players %s in PlaySoundForAllPlayers"), *SoundName.ToString());

	PlaySoundMulticast(SoundData->SoundAsset, SoundData->SoundStartTime, SoundLocation, SoundPowerLevel);
}

void APGSoundManager::PlaySoundWithNoise_Implementation(FName SoundName, FVector SoundLocation, uint8 SoundPowerLevel, bool bIntensedSound)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundData from %s"), *SoundName.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Start sound multicast %s in PlaySoundWithNoise"), *SoundName.ToString());
	// Play sound for all players by execute PlaySoundMulticast.
	PlaySoundMulticast(SoundData->SoundAsset, SoundData->SoundStartTime, SoundLocation, SoundPowerLevel);

	// Make noise for enemy AI chase sound.
	float SoundRange = 200 * SoundPowerLevel * SoundPowerLevel;

	// When bIntensedSound is true, the sound power increases but the range remains the same.
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

void APGSoundManager::PlaySoundMulticast_Implementation(USoundBase* SoundAsset, float SoundStartTime, FVector SoundLocation, uint8 SoundPowerLevel)
{
	// Sets the audible range and the radius where sound attenuation starts.
	float AttenuationExtentRange, AttenuationFalloffDistance;
	AttenuationExtentRange = 200 * SoundPowerLevel * SoundPowerLevel;
	AttenuationFalloffDistance = 200 * (SoundPowerLevel - 1) * (SoundPowerLevel - 1);

	// AttenuationShapeExtents.X is the sphere radius of attenuation.
	BaseSoundAttenuation->Attenuation.AttenuationShapeExtents.X = AttenuationExtentRange;
	BaseSoundAttenuation->Attenuation.FalloffDistance = AttenuationFalloffDistance;

	UE_LOG(LogTemp, Log, TEXT("Play sound %s in PlaySoundMulticast"), *SoundAsset->GetName());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundAsset, SoundLocation, 1.0f, 1.0f, SoundStartTime, BaseSoundAttenuation);
}

