// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGPhotoTypes.generated.h"

// 백의 자리: 카테고리
// 1xx = Monster, 2xx = Anomaly

// 십의 자리: 종류
// Monster: 11x=Blind, 12x=Charger, 13x=Ghost
// Anomaly: 21x=Mannequin, 22x=WindowBlood, 23x=RockingChair, 24x=Phone

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

    // Anomaly
    constexpr int32 Mannequin = 210;
    constexpr int32 WindowBlood = 220;
    constexpr int32 RockingChair = 230;
    constexpr int32 Phone = 240;
    
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

    struct FPhotoIDEntry
    {
        int32 ID;
        int32 Score;
        bool bMultiplayerOnly;
    };

    // 값 변경시 피사체 클래스의 GetPhotoSubjectInfo도 수정 필요
    inline const TArray<FPhotoIDEntry>& GetAllEntries()
    {
        // 멀티 최대 145
        // 싱글 최대 135
        static const TArray<FPhotoIDEntry> Entries = 
        {
            { Blind_Exploring, 10, false },
            { Blind_Chasing, 10, false },
            { Blind_Attacking, 5, true },
            { Charger_Exploring, 10, false },
            { Charger_Staring, 10, false },
            { Charger_Attacking, 5, false },
            { Charger_Killing, 5, true },
            { Ghost_Exploring, 10, false },
            { Ghost_Chasing, 10, false },
            { WindowBlood, 10, false },
            { RockingChair, 10, false },
            { Phone, 10, false },
            { Room_Charger, 10, false },
            { Room_Blind, 10, false },
            { Room_Ghost, 10, false },
            { Room_Elevator, 10, false },
        };
        return Entries;
    }

    inline int32 GetMaxPossibleScore(bool bSinglePlay)
    {
        int32 Total = 0;
        for (const FPhotoIDEntry& Entry : GetAllEntries())
        {
            if (bSinglePlay && Entry.bMultiplayerOnly)
            {
                continue;
            }
            Total += Entry.Score;
        }
        return Total;
    }
}

namespace PhotoGrade
{
    enum class EGrade : uint8 { F, D, C, B, A, S };

    FORCEINLINE EGrade GetGradeEnum(int32 Score, int32 MaxScore)
    {
        if (MaxScore <= 0)
        {
            return EGrade::F;
        }

        const float Ratio = (float)Score / (float)MaxScore;

        // Single: 118.8(120), Multi: 127.6(130)
        if (Ratio >= 0.88)
        {
            return EGrade::S;
        }

        // Single: 87.75(90), Multi: 94.25(95)
        if (Ratio >= 0.65)
        {
            return EGrade::A;
        }

        // Single: 59.4(60), Multi: 63.8(65)
        if (Ratio >= 0.44)
        {
            return EGrade::B;
        }

        // Single: 36.45(40), Multi: 39.15(40)
        if (Ratio >= 0.27)
        {
            return EGrade::C;
        }

        // Single: 17.55(20), Multi: 18.85(20)
        if (Ratio >= 0.13)
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

    FORCEINLINE FString GetGrade(int32 Score, int32 MaxScore) { return GetGradeText(GetGradeEnum(Score, MaxScore)); }
    FORCEINLINE int32 GetGradeXPFromScore(int32 Score, int32 MaxScore) { return GetGradeXP(GetGradeEnum(Score, MaxScore)); }
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