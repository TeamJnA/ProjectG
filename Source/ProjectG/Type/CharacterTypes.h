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

UENUM(BlueprintType)
enum class EGameTeam : uint8
{
	Player	UMETA(DisplayName = "Player"),
	AI		UMETA(DisplayName = "AI")
};

UENUM(BlueprintType)
enum class EHandPoseType : uint8
{
	Default,
	Grab,
	Pinch,
	Count UMETA(Hidden)
};