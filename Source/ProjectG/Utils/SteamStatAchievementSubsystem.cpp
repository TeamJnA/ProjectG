// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/SteamStatAchievementSubsystem.h"
#include "steam/steam_api.h"

bool USteamStatAchievementSubsystem::SaveStat(const FString& APIName, int32 Value)
{
    ISteamUserStats* Stats = SteamUserStats();
    if (!Stats || !Stats->SetStat(TCHAR_TO_UTF8(*APIName), Value))
    {
        return false;
    }

    return Stats->StoreStats();
}

bool USteamStatAchievementSubsystem::UnlockAchievement(const FString& APIName)
{
    ISteamUserStats* Stats = SteamUserStats();
    if (!Stats || !Stats->SetAchievement(TCHAR_TO_UTF8(*APIName)))
    {
        return false;
    }

    return Stats->StoreStats();
}
