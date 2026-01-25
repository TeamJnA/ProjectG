// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGGameTypes.generated.h"

UENUM(BlueprintType)
enum class EExitPointType : uint8
{
    None        UMETA(DisplayName = "None"),
    IronDoor    UMETA(DisplayName = "Iron Door"),
    Elevator    UMETA(DisplayName = "Elevator"),
    Count
};