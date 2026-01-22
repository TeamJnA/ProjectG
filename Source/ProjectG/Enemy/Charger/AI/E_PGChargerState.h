#pragma once

#include "CoreMinimal.h"
#include "E_PGChargerState.generated.h"

/**
 * Custom enum to demonstrate how to expose enums to Blueprints.
 */

UENUM(BlueprintType)
enum class E_PGChargerState : uint8
{
	Exploring	UMETA(DisplayName = "Exploring"), // 탐색
	Staring		UMETA(DisplayName = "Staring"),  // 응시
	Adjusting	UMETA(DisplayName = "Adjusting"), // 돌진 각 조절
	Attacking	UMETA(DisplayName = "Attacking"), // 공격(돌진)
	Killing		UMETA(DisplayName = "Killing")    // 플레이어 공격
};