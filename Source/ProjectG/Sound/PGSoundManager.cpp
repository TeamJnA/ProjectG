// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

// This header is required to use ReportNoiseEvent
#include "Perception/AISense_Hearing.h"

// Sets default values for this component's properties
APGSoundManager::APGSoundManager()
{
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

			// Cast the data from pointer(uint8*) to PGSoundPlayData
			FPGSoundPlayData* RowData = reinterpret_cast<FPGSoundPlayData*>(DataTableRow.Value);

			if (RowData && RowData->SoundAsset)
			{
				UE_LOG(LogTemp, Log, TEXT("Add SoundData to SoundDataMap : %s"), *RowName.ToString());
				SoundDataMap.Add(RowName, FPGSoundPlayData(RowData->SoundAsset, RowData->SoundLevel, RowData->SoundStartTime));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid Row or missing SoundAsset for row: %s"), *RowName.ToString());
			}
		}
	}
	
}

void APGSoundManager::BeginPlay()
{
	Super::BeginPlay();
}

void APGSoundManager::PlaySoundForSelf(FName SoundName)
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

void APGSoundManager::PlaySoundForAllPlayers_Implementation(FName SoundName, FVector SoundLocation)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundData from %s"), *SoundName.ToString());
		return;
	}

	// Play sound for all players by execute PlaySoundMulticast.
	UE_LOG(LogTemp, Log, TEXT("Play sound for all players %s in PlaySoundForAllPlayers"), *SoundName.ToString());

	PlaySoundMulticast(SoundData->SoundAsset, SoundData->SoundStartTime, SoundLocation, SoundData->SoundLevel);
}

void APGSoundManager::PlaySoundWithNoise_Implementation(FName SoundName, FVector SoundLocation, bool bIntensedSound)
{
	FPGSoundPlayData* SoundData = SoundDataMap.Find(SoundName);
	if (!SoundData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundData from %s"), *SoundName.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Start sound multicast %s in PlaySoundWithNoise"), *SoundName.ToString());
	// Play sound for all players by execute PlaySoundMulticast.
	PlaySoundMulticast(SoundData->SoundAsset, SoundData->SoundStartTime, SoundLocation, SoundData->SoundLevel);


	// 만약 sound의 Level이 0일  경우, 아래 작업은 진행할 필요가 없음.
	if (SoundData->SoundLevel == 0)
	{
		return;
	}

	// Make noise for enemy AI chase sound.
	int SoundPowerLevel = SoundData->SoundLevel;

	const int SoundRange = 200 * SoundPowerLevel * SoundPowerLevel;

	// When bIntensedSound is true, the sound power increases but the range remains the same.
	if (bIntensedSound)
	{
		SoundPowerLevel++;
	}

	UE_LOG(LogTemp, Log, TEXT("Make Noise Level %d. [ Location : %s ], [ Range : %d ]"), SoundPowerLevel, *SoundLocation.ToString(), SoundRange);

	UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),    
		SoundLocation,
		SoundPowerLevel,
        this, 
		SoundRange,
		SoundName
    );

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

