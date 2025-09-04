#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PGSoundPlayData.generated.h"

USTRUCT(BlueprintType)
struct PROJECTG_API FPGSoundPlayData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> SoundAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 SoundLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoundStartTime;

	FPGSoundPlayData()
		: SoundAsset(nullptr)
		, SoundLevel(1)
		, SoundStartTime(0.f)
	{
	}

	FPGSoundPlayData(USoundBase* InSoundAsset, int32 InSoundLevel ,float InStartTime)
		: SoundAsset(InSoundAsset)
		, SoundLevel(InSoundLevel)
		, SoundStartTime(InStartTime)
	{
	}
};