#pragma once

#include "CoreMinimal.h"
#include "PGStartRoomTypes.generated.h"

UENUM(BlueprintType)
enum class EStartRoomExit : uint8
{
    ExitDir0    UMETA(DisplayName = "ExitDir0"),
    ExitDir1    UMETA(DisplayName = "ExitDir1"),
    ExitDir2    UMETA(DisplayName = "ExitDir2"),
    ExitDir3    UMETA(DisplayName = "ExitDir3"),
    ExitDir4    UMETA(DisplayName = "ExitDir4"),
    None        UMETA(DisplayName = "None")
};
