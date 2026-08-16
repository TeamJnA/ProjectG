// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Type/PGGameTypes.h"
#include "PGHelperTypes.generated.h"


USTRUCT(BlueprintType)
struct FPGHelperRequiredItem
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    EPGExitItemType ItemType = EPGExitItemType::None;

    UPROPERTY(EditDefaultsOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly)
    int32 RequiredCount = 1;
};

USTRUCT(BlueprintType)
struct FPGHelperEntryRow : public FTableRowBase
{
    GENERATED_BODY()

    // TAB 위젯에 표시될 이름
    UPROPERTY(EditDefaultsOnly)
    FText DisplayName;

    // 필요 아이템 아이콘 리스트
    UPROPERTY(EditDefaultsOnly)
    TArray<FPGHelperRequiredItem> RequiredItems;

    // 카테고리 내 정렬 시드. Exit는 MainExit=0, Elevator=1처럼 고정
    UPROPERTY(EditDefaultsOnly)
    int32 DisplayOrder = 0;

    // 촬영하지 않아도 기본 노출할지 여부
    UPROPERTY(EditDefaultsOnly)
    bool bDefaultVisible = false;
};

USTRUCT(BlueprintType)
struct FPGEnemyCatalogRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
    FText TooltipText;
};
