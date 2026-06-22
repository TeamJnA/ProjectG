// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PGRankTypes.generated.h"


USTRUCT(BlueprintType)
struct FPGRankRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rank")
	FText RankTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rank")
	int64 RequiredTotalXP = 0;
};

USTRUCT(BlueprintType)
struct FPGRankProgress
{
	GENERATED_BODY()

	int32 RankIndex = 0;
	FText RankTitle;
	int64 CurrentRankFloorXP = 0;   // 현재 등급 시작 XP
	int64 NextRankXP = 0;           // 다음 등급 요구 XP (maxed면 floor와 동일)
	bool  bIsMaxRank = false;
};
