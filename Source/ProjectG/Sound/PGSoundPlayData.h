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
	float SoundStartTime;

	FPGSoundPlayData()
		: SoundAsset(nullptr)
		, SoundStartTime(0.f)
	{
	}

	FPGSoundPlayData(USoundBase* InSoundAsset, float InStartTime)
		: SoundAsset(InSoundAsset)
		, SoundStartTime(InStartTime)
	{
	}
};