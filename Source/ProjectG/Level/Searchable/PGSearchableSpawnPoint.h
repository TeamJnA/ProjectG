// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "PGSearchableSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class ESearchableType : uint8
{
	Cabinet		UMETA(DisplayName = "Cabinet"),
	Dresser1	UMETA(DisplayName = "Dresser Type 1"),
	Dresser2	UMETA(DisplayName = "Dresser Type 2"),
	Dresser3 UMETA(DisplayName = "Dresser Type 3"),
	Shelf		UMETA(DisplayName = "Shelf"),
	Table UMETA(DisplayName = "Table"),
	OldDrawer UMETA(DisplayName = "OldDrawer")
};

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UPGSearchableSpawnPoint : public UArrowComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE ESearchableType GetSearchableType() const { return SearchableType; };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ESearchableType SearchableType = ESearchableType::Cabinet;
};
