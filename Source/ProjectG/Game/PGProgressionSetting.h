// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGProgressionSetting.generated.h"

class UDataTable;

/**
 * 
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PG Progression"))
class PROJECTG_API UPGProgressionSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Rank")
	TSoftObjectPtr<UDataTable> RankTable;

	UPROPERTY(config, EditAnywhere, Category = "Difficulty")
	TSoftObjectPtr<UDataTable> DifficultyTable;

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
