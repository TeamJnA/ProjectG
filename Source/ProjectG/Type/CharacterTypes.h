#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

/**
 * Custom enum to demonstrate how to expose enums to Blueprints.
 */

UENUM(BlueprintType)
enum class EHandActionMontageType : uint8
{
	Pick,
	Change
};