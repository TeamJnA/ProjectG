#pragma once

#include "CoreMinimal.h"

#include "GameFramework/OnlineReplStructs.h"

#include "CharacterTypes.generated.h"

/**
 * Custom enum to demonstrate how to expose enums to Blueprints.
 */

UENUM(BlueprintType)
enum class EHandActionMontageType : uint8
{
	Pick,
	Change,
	Drop
};

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl PlayerNetId;

	UPROPERTY(BlueprintReadOnly)
	bool bIsHost = false;
};