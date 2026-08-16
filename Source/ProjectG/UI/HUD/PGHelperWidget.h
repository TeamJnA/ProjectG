// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Type/PGHelperTypes.h"

#include "PGHelperWidget.generated.h"

class UHorizontalBox;
class UDataTable;
class UPGHelperExitEntryWidget;
class APGExitPointBase;

USTRUCT()
struct FPGHelperPendingRow
{
    GENERATED_BODY()

    UPROPERTY()
    FPGHelperEntryRow Row;

    UPROPERTY()
    TMap<EPGExitItemType, int32> UnlockedCounts;

    UPROPERTY()
    int32 SpeciesKey = 0;

    UPROPERTY()
    bool bDepleted = false;
};

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGHelperWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // Helper 위젯 열기 -> Auto Close(5s)
    void OpenWithAutoClose();

    // 외부에서 강제로 닫기
    void CloseIfOpen();
    void CloseAndCollapse();
    void RestoreFromCameraMode();
    void ForceClose();

    void NotifyCapturedChanged();
    void NotifyExitLockChanged();

    FORCEINLINE bool IsOpen() const { return bIsOpen; }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void HandleSlideAnimFinished();

    void SnapToClosedState();

    // HelperWidget 갱신 
    // Refresh->연출까지 재시작
    // UpdateInPlace->연출이 끝난 상태에서 연출 없이 갱신
    void Refresh();
    void UpdateInPlace();

    bool IsIntroPlaying() const;
    void BuildExitByKeyMap(TMap<int32, APGExitPointBase*>& OutExitByKey) const;
    void ResolvePendingRows();

    // Typewriter, Icon Image 등장 연출
    UFUNCTION()
    void AppearNextRow();

    TArray<TWeakObjectPtr<UPGHelperExitEntryWidget>> ActiveEntries;

    UPROPERTY()
    TArray<FPGHelperPendingRow> PendingRows;

    UPROPERTY(EditDefaultsOnly, Category = "Helper")
    TObjectPtr<UDataTable> CatalogTable;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> HelperListRoot;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidget> HelperPeek;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> ExitListBox;

    UPROPERTY(EditDefaultsOnly, Category = "Helper")
    TSubclassOf<UPGHelperExitEntryWidget> ExitEntryWidgetClass;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> SlideInAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> SlideOutAnim;

    UPROPERTY(EditDefaultsOnly, Category = "Helper")
    FVector2D ListClosedTranslation = FVector2D(0.0f, -150.0f);

    FTimerHandle AutoCloseTimerHandle;
    FTimerHandle RowAppearTimerHandle;

    UPROPERTY(EditDefaultsOnly)
    float AutoCloseDelay = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Helper")
    float RowAppearInterval = 0.3f;

    int32 NextPendingIndex = 0;
    bool bIsOpen = false;
};
