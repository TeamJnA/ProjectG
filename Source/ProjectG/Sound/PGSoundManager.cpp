// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/PGSoundManager.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Perception/AISense_Hearing.h"

DEFINE_LOG_CATEGORY(LogSoundManager);

APGSoundManager::APGSoundManager()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	bDebugSoundRange = false;

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

		// Get a map of all the rows in the DataTable as a pointer to the row data (uint8*)
		const TMap<FName, uint8*>& DataTableRows = SoundDataTable->GetRowMap();

		for (const TPair<FName, uint8*>& DataTableRow : DataTableRows)
		{
			FName RowName = DataTableRow.Key;

			// Cast the data from pointer(uint8*) to PGSoundPlayData
			FPGSoundPlayData* RowData = reinterpret_cast<FPGSoundPlayData*>(DataTableRow.Value);

			if (RowData && RowData->SoundAsset)
			{
				UE_LOG(LogSoundManager, Log, TEXT("Add SoundData to SoundDataMap : %s"), *RowName.ToString());
				SoundDataMap.Add(RowName, FPGSoundPlayData(RowData->SoundAsset, RowData->SoundLevel, RowData->SoundStartTime));
			}
			else
			{
				ensureMsgf(false, TEXT("Invalid row or missing soundasset in SoundDataTable."));
			}
		}
	}
}

void APGSoundManager::PlaySoundForSelf(const FName& SoundName)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogSoundManager, Warning, TEXT("Cannot find SoundData from [%s]"), *SoundName.ToString());
		return;
	}

	UGameplayStatics::PlaySound2D(GetWorld(), SoundData->SoundAsset, 1.0f, 1.0f, SoundData->SoundStartTime);
}

void APGSoundManager::PlaySoundForAllPlayers_Implementation(const FName& SoundName, const FVector& SoundLocation)
{
	PlaySoundMulticast(SoundName, SoundLocation);
}

void APGSoundManager::PlaySoundWithNoise_Implementation(const FName& SoundName, const FVector& SoundLocation, bool bIntensedSound)
{
	PlaySoundMulticast(SoundName, SoundLocation);


	// Make noise for enemy AI chase sound.

	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogSoundManager, Warning, TEXT("Cannot find SoundData from [%s]"), *SoundName.ToString());
		return;
	}

	// There is no Noise when sound level is 0.
	if (SoundData->SoundLevel == 0)
	{
		return;
	}
	
	// When bIntensedSound is true, the sound power increases but the range remains the same.
	const float SoundPowerLevel = SoundData->SoundLevel + (bIntensedSound ? 1 : 0);

	// 실제 Sound Range = 200 * SoundPowerLevel * SoundPowerLevel. 
	// 여기서 Loudness를 나누어야 한다. Sound Range * Loudness만큼 소리가 들리는 범위임.
	const float SoundRange = 200 * SoundPowerLevel; // 200 * SoundPowerLevel * SoundPowerLevel / SoundPowerLevel(Loudness)

	UE_LOG(LogSoundManager, Log, TEXT("Make Noise Level %f. [ Location : %s ], [ Range : %f ]"), SoundPowerLevel, *SoundLocation.ToString(), SoundRange);

	UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),    
		SoundLocation,
		SoundPowerLevel,
        this, 
		SoundRange,
		SoundName
    );

#if WITH_EDITOR
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
			DrawDebugSphere(GetWorld(), SoundLocation, SoundRange * SoundPowerLevel, 8, FColor::Yellow, false, 3.0f, 0U, 3.0f);
		}
	}
#endif
}

void APGSoundManager::PlaySoundMulticast_Implementation(const FName& SoundName, const FVector& SoundLocation)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogSoundManager, Warning, TEXT("Cannot find SoundData from [%s]"), *SoundName.ToString());
		return;
	}

	// Sets the audible range and the radius where sound attenuation starts.
	const float SoundPowerLevel = SoundData->SoundLevel;

	const float AttenuationExtentRange = 200 * SoundPowerLevel * SoundPowerLevel;
	const float AttenuationFalloffDistance = 200 * (SoundPowerLevel - 1) * (SoundPowerLevel - 1);

	// AttenuationShapeExtents.X is the sphere radius of attenuation.
	BaseSoundAttenuation->Attenuation.AttenuationShapeExtents.X = AttenuationExtentRange;
	BaseSoundAttenuation->Attenuation.FalloffDistance = AttenuationFalloffDistance;

	UE_LOG(LogSoundManager, Log, TEXT("Play sound %s in PlaySoundMulticast"), *SoundName.ToString());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundData->SoundAsset, SoundLocation, 1.0f, 1.0f, SoundData->SoundStartTime, BaseSoundAttenuation);
}

