// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PGSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	int64 TotalXP = 0;

	UPROPERTY()
	int32 RankIndex = 0;

	UPROPERTY()
	int32 GamesCompleted = 0;

	UPROPERTY()
	FString Signature;

	FString ComputeSignature() const;
	bool VerifySignature() const;
};
