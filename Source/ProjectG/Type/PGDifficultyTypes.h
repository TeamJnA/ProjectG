// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PGDifficultyTypes.generated.h"

UENUM(BlueprintType)
enum class EPGDifficulty : uint8
{
	Normal,
	Hard
};

USTRUCT(BlueprintType)
struct FPGDifficultySettings : public FTableRowBase
{
	GENERATED_BODY()

	// 걷기 발소리 노이즈 레벨 (절대값)
	UPROPERTY(EditAnywhere)
	uint8 WalkFootstepNoiseLevel = 2;

	// Charger 시야 범위
	UPROPERTY(EditAnywhere)
	float ChargerSightRangeMultiplier = 1.0f;

	// 공격 전환 누적시간 배율 (작을수록 빠름)
	UPROPERTY(EditAnywhere)
	float ChargerStareTimeMultiplier = 1.0f;

	// MaxSanityDecreaseInterval = Sanity가 15 감소하는 시간 (= GE Period × 15)
	// Normal: 15×15=225 / Hard: 10×15=150.
	// Sanity GE Period 바꾸면 여기도 동기화 필요.
	UPROPERTY(EditAnywhere)
	float MaxSanityDecreaseInterval = 225.0f;

	// 맵 데칼 스폰 강도 0~1
	UPROPERTY(EditAnywhere)
	float HorrorDecorIntensity = 0.5f;

	// XP 배율
	UPROPERTY(EditAnywhere)
	float XPMultiplier = 1.0f;

	// 고려 요소 (플래그만, 로직 나중)
	UPROPERTY(EditAnywhere)
	bool bEnableLanternControl = false;
};