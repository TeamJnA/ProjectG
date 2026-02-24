// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGGameTypes.generated.h"

UENUM(BlueprintType)
enum class EExitPointType : uint8
{
    None        UMETA(DisplayName = "None"),
    IronDoor    UMETA(DisplayName = "Iron Door"),
    Elevator    UMETA(DisplayName = "Elevator"),
    Count
};

UENUM(BlueprintType)
enum class ECleanupActionType : uint8
{
	None,
	Solo_ReturnToMainMenu, // [Client] 혼자 나가기
	Solo_QuitToDesktop,    // [Client] 혼자 종료
	Mass_ServerTravel,     // [Server] 레벨 이동 (Lobby <-> Main)
	Mass_KickForDestroy    // [Server] 모두 종료 (Lobby/MainLevel -> MainMenu/Desktop)
};