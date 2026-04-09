// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGPhotoTypes.generated.h"

// 백의 자리: 카테고리
// 1xx = Monster, 2xx = Anomaly

// 십의 자리: 종류
// Monster: 11x=Blind, 12x=Charger, 13x=Ghost
// Anomaly: 21x=Mannequin, 22x=WindowBlood

// 일의 자리: 상태
// Monster: x1=Exploring, x2=Investigating, x3=Chasing, x4=Attacking
// Anomaly: x0 (상태 없음)

namespace PhotoID
{
    // Blind
    constexpr int32 Blind_Exploring = 111;
    constexpr int32 Blind_Chasing = 112;

    // Charger
    constexpr int32 Charger_Exploring = 121;
    constexpr int32 Charger_Staring = 122;
    constexpr int32 Charger_Attacking = 123;

    // Ghost
    constexpr int32 Ghost_Exploring = 131;
    constexpr int32 Ghost_Chasing = 132;

    // MirrorGhost
    constexpr int32 MirrorGhost = 140;

    // Anomaly
    constexpr int32 Mannequin = 210;
    constexpr int32 WindowBlood = 220;
    
    // Room
    constexpr int32 Room_Charger_1 = 310;
    constexpr int32 Room_Charger_2 = 311;
    constexpr int32 Room_Charger_3 = 312;
    constexpr int32 Room_Blind = 320;
    constexpr int32 Room_Ghost = 330;
    constexpr int32 Room_Elevator = 340;

    // 카테고리 판별
    FORCEINLINE bool IsMonster(int32 ID) { return ID / 100 == 1; }
    FORCEINLINE bool IsAnomaly(int32 ID) { return ID / 100 == 2; }
    FORCEINLINE bool IsRoom(int32 ID) { return ID / 100 == 3; }
    FORCEINLINE int32 GetRoomCategory(int32 ID) { return ID / 10; }
    FORCEINLINE bool IsSameRoomCategory(int32 A, int32 B) { return GetRoomCategory(A) == GetRoomCategory(B); }
}

namespace PhotoGrade
{
    FORCEINLINE FString GetGrade(int32 Score)
    {
        if (Score >= 1000)
        {
            return TEXT("S");
        }

        if (Score >= 700)
        {
            return TEXT("A");
        }

        if (Score >= 500)
        {
            return TEXT("B");
        }

        if (Score >= 300)
        {
            return TEXT("C");
        }

        if (Score >= 100)
        {
            return TEXT("D");
        }

        return TEXT("F");
    }
}

USTRUCT(BlueprintType)
struct FPhotoSubjectInfo
{
    GENERATED_BODY()

    UPROPERTY()
    int32 SubjectID = 0;

    UPROPERTY()
    int32 ScoreValue = 0;

    FPhotoSubjectInfo() {}
    FPhotoSubjectInfo(int32 InID, int32 InScore)
        : SubjectID(InID), ScoreValue(InScore) {}
};

USTRUCT()
struct FPhotoSpotConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    int32 PhotoID = 0;

    UPROPERTY(EditDefaultsOnly)
    int32 PhotoScore = 100;

    UPROPERTY(EditDefaultsOnly)
    FVector Offset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly)
    FVector BoxExtent = FVector(32.0f);
};