// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "PGGimmickSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class EGimmickType : uint8
{
    ArmorStand   UMETA(DisplayName = "Armor Stand"),
    Phone        UMETA(DisplayName = "Phone"),
    RockingChair UMETA(DisplayName = "Rocking Chair")
};

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UPGGimmickSpawnPoint : public UArrowComponent
{
    GENERATED_BODY()

public:
    FORCEINLINE EGimmickType GetGimmickType() const { return GimmickType; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EGimmickType GimmickType = EGimmickType::ArmorStand;	
};
