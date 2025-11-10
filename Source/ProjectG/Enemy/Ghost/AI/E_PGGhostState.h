#pragma once

#include "CoreMinimal.h"
#include "E_PGGhostState.generated.h"

/**
 * Custom enum to demonstrate how to expose enums to Blueprints.
 */

UENUM(BlueprintType)
enum class E_PGGhostState : uint8
{
	Exploring	UMETA(DisplayName = "Exploring"), // 탐색
	Tracking	UMETA(DisplayName = "Tracking"),  // 추적 
	Chasing		UMETA(DisplayName = "Chasing"),   // 추격
	Waiting		UMETA(DisplayName = "Waiting")    // 추격 후 대기
};