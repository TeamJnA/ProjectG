// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "PGLooseItemSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class ELooseItemType : uint8
{
	Lantern		UMETA(DisplayName = "Lantern")
};

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UPGLooseItemSpawnPoint : public UArrowComponent
{
	GENERATED_BODY()
	
public:
	FORCEINLINE ELooseItemType GetELooseItemType() const { return LooseItemType; };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ELooseItemType LooseItemType = ELooseItemType::Lantern;
};
