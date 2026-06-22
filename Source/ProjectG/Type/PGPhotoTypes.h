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
    constexpr int32 Blind_Attacking = 113;

    // Charger
    constexpr int32 Charger_Exploring = 121;
    constexpr int32 Charger_Staring = 122;
    constexpr int32 Charger_Attacking = 123;
    constexpr int32 Charger_Killing = 124;

    // Ghost
    constexpr int32 Ghost_Exploring = 131;
    constexpr int32 Ghost_Chasing = 132;

    // MirrorGhost
    constexpr int32 MirrorGhost = 140;

    // Anomaly
    constexpr int32 Mannequin = 210;
    constexpr int32 WindowBlood = 220;
    
    // Room
    constexpr int32 Room_Charger = 310;
    constexpr int32 Room_Blind = 320;
    constexpr int32 Room_Ghost = 330;
    constexpr int32 Room_Elevator = 340;
    constexpr int32 ExitDoor = 350;

    // 카테고리 판별
    FORCEINLINE bool IsMonster(int32 ID) { return ID / 100 == 1; }
    FORCEINLINE bool IsAnomaly(int32 ID) { return ID / 100 == 2; }
    FORCEINLINE bool IsRoom(int32 ID) { return ID / 100 == 3; }
    FORCEINLINE int32 GetSpeciesKey(int32 ID) { return IsRoom(ID) ? ID : (ID / 10); }
    FORCEINLINE int32 GetCategory(int32 ID) { return ID / 100; }
}

namespace PhotoGrade
{
    enum class EGrade : uint8 { F, D, C, B, A, S };

    FORCEINLINE EGrade GetGradeEnum(int32 Score)
    {
        if (Score >= 120)
        {
            return EGrade::S;
        }

        if (Score >= 100)
        {
            return EGrade::A;
        }

        if (Score >= 60)
        {
            return EGrade::B;
        }

        if (Score >= 40)
        {
            return EGrade::C;
        }

        if (Score >= 20)
        {
            return EGrade::D;
        }

        return EGrade::F;
    }

    FORCEINLINE FString GetGradeText(EGrade Grade)
    {
        switch (Grade)
        {
            case EGrade::S:
            {
                return TEXT("S");
            }

            case EGrade::A:
            {
                return TEXT("A");
            }

            case EGrade::B:
            {
                return TEXT("B");
            }

            case EGrade::C:
            {
                return TEXT("C");
            }

            case EGrade::D:
            {
                return TEXT("D");
            }

            default:
            {
                return TEXT("F");
            }
        }
    }

    FORCEINLINE int32 GetGradeXP(EGrade Grade)
    {
        switch (Grade)
        {
            case EGrade::S:
            {
                return 800;
            }

            case EGrade::A:
            {
                return 500;
            }

            case EGrade::B:
            {
                return 300;
            }

            case EGrade::C:
            {
                return 200;
            }

            case EGrade::D:
            {
                return 150;
            }

            default:
            {
                return 100;   // F
            }
        }
    }

    FORCEINLINE FString GetGrade(int32 Score) { return GetGradeText(GetGradeEnum(Score)); }
    FORCEINLINE int32 GetGradeXPFromScore(int32 Score) { return GetGradeXP(GetGradeEnum(Score)); }
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
struct FPhotoCaptureResult
{
    GENERATED_BODY()

    UPROPERTY()
    int32 SubjectID = 0;

    UPROPERTY()
    int32 AwardedScore = 0;

    UPROPERTY()
    bool bNewRecord = false;

    UPROPERTY()
    bool bNewSpecies = false;
};

USTRUCT()
struct FCaptureLogLine
{
    GENERATED_BODY()

    UPROPERTY()
    FText Text;

    UPROPERTY()
    bool bValid = false;
};

USTRUCT()
struct FPhotoSpotConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    int32 PhotoID = 0;

    UPROPERTY(EditDefaultsOnly)
    int32 PhotoScore = 10;

    UPROPERTY(EditDefaultsOnly)
    FVector Offset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly)
    FVector BoxExtent = FVector(32.0f);
};