// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Type/PGDifficultyTypes.h"
#include "PGSessionTypes.generated.h"

/** 세션 최대 인원 (고정) */
static constexpr int32 PG_MAX_SESSION_PLAYERS = 4;

/** 세션 표시 이름 최대 길이 */
static constexpr int32 PG_MAX_SESSION_NAME_LENGTH = 32;

/**
 * 세션 생성 시 선택한 옵션
 * DisplayName -> 세션 목록에 표시되는 이름 (NAME_GameSession과 무관)
 */
USTRUCT(BlueprintType)
struct FPGHostSessionOptions
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    bool bIsInviteOnly = false;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    bool bIsSinglePlay = false;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    EPGDifficulty Difficulty = EPGDifficulty::Normal;
};
