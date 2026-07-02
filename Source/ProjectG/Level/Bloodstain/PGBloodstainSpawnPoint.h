// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "PGBloodstainSpawnPoint.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UPGBloodstainSpawnPoint : public UArrowComponent
{
	GENERATED_BODY()
	
public:
	UPGBloodstainSpawnPoint();

	UPROPERTY(EditAnywhere, Category = "Bloodstain")
	TSubclassOf<AActor> DecalClass;

	UPROPERTY(EditAnywhere, Category = "Bloodstain")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Bloodstain")
	FVector DecalSize = FVector(20.0f, 100.0f, 100.0f);
};
