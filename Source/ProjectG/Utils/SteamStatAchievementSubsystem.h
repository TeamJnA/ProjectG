// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SteamStatAchievementSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API USteamStatAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Steam")
	bool SaveStat(const FString& APIName, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Steam")
	bool UnlockAchievement(const FString& APIName);
};
