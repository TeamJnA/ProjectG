// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGAlertContainerWidget.generated.h"

class UPGHelperWidget;
class UPGPhotoAlertWidget;
class UPGExitToastWidget;
class UPGEnemyToastWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAlertContainerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void OpenHelper();
	void CloseHelper();
	void ResetForCleanup();
	void NotifyHelperCapturedChanged();
	void NotifyHelperExitLockChanged();
	bool IsHelperOpen() const;

	void SetPhotoAlertActive(bool bActive);

	void RequestExitToast();
	void DismissExitToast();

	void PlayEnemyToast(const FText& Text);

	void OnEnterCameraMode();
	void OnExitCameraMode();
	void OnExitCameraMode_EnemyToast();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGHelperWidget> HelperWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGPhotoAlertWidget> PhotoAlertWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGExitToastWidget> ExitToastWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGEnemyToastWidget> EnemyToastWidget;

	// ExitToast는 일반 모드에서 항상 떠 있어야 함 (HelperWidget 오픈하면 제거)
	// 카메라 모드에서 갱신 -> 복귀 후 노출
	bool bExitActive = false;
	bool bExitPending = false;
	bool bInCameraMode = false;
};
