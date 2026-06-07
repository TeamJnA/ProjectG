// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGAlertContainerWidget.h"
#include "UI/HUD/PGHelperWidget.h"
#include "UI/HUD/PGPhotoAlertWidget.h"
#include "UI/HUD/PGExitToastWidget.h"
#include "UI/HUD/PGEnemyToastWidget.h"


void UPGAlertContainerWidget::OpenHelper()
{
	if (!HelperWidget)
	{
		return;
	}
	HelperWidget->OpenWithAutoClose();

	// Helper 오픈 -> ExitToast 페이드아웃 + 상태 해제
	if (bExitActive && ExitToastWidget)
	{
		ExitToastWidget->DismissToast();
	}

	bExitActive = false;
	bExitPending = false;
}

void UPGAlertContainerWidget::CloseHelper()
{
	if (HelperWidget)
	{
		HelperWidget->CloseIfOpen();
	}
}

void UPGAlertContainerWidget::ResetForCleanup()
{
	if (HelperWidget)
	{
		HelperWidget->ForceClose();
	}

	bExitActive = false;
	bExitPending = false;
	if (ExitToastWidget)
	{
		ExitToastWidget->HideImmediate();
	}

	if (EnemyToastWidget)
	{
		EnemyToastWidget->ResetToast();
	}

	if (PhotoAlertWidget)
	{
		PhotoAlertWidget->RestoreFromCameraMode();
	}
	bInCameraMode = false;
}

bool UPGAlertContainerWidget::IsHelperOpen() const
{
	return HelperWidget && HelperWidget->IsOpen();
}

void UPGAlertContainerWidget::SetPhotoAlertActive(bool bActive)
{
	if (PhotoAlertWidget)
	{
		PhotoAlertWidget->SetPhotoAlertActive(bActive);
	}
}

void UPGAlertContainerWidget::RequestExitToast()
{
	// Helper 열려있으면 알림 x
	if (IsHelperOpen())
	{
		return;
	}

	// 카메라 모드 중에 보류
	if (bInCameraMode)
	{
		bExitPending = true;
		return;
	}

	// 이미 ExitToast가 있으면 x
	if (bExitActive)
	{
		return;
	}

	// Toast 생성
	bExitActive = true;
	if (ExitToastWidget)
	{
		ExitToastWidget->ShowToast();
	}
}

void UPGAlertContainerWidget::DismissExitToast()
{
	bExitActive = false;
	bExitPending = false;
	if (ExitToastWidget)
	{
		ExitToastWidget->DismissToast();
	}
}

void UPGAlertContainerWidget::PlayEnemyToast(const FText& TooltipText)
{
	if (EnemyToastWidget)
	{
		EnemyToastWidget->PlayToast(TooltipText);
	}
}

void UPGAlertContainerWidget::OnEnterCameraMode()
{
	bInCameraMode = true;

	if (HelperWidget)
	{
		HelperWidget->CloseAndCollapse();
	}

	if (PhotoAlertWidget)
	{
		PhotoAlertWidget->CollapseForCameraMode();
	}

	if (bExitActive && ExitToastWidget)
	{
		ExitToastWidget->HideImmediate();
	}

	if (EnemyToastWidget)
	{
		EnemyToastWidget->HideBrieflyForTransition();
	}
}

void UPGAlertContainerWidget::OnExitCameraMode()
{
	bInCameraMode = false;

	if (HelperWidget)
	{
		HelperWidget->RestoreFromCameraMode();
	}

	if (PhotoAlertWidget)
	{
		PhotoAlertWidget->RestoreFromCameraMode();
	}

	if (bExitPending)
	{
		bExitActive = true;
		bExitPending = false;
	}

	if (bExitActive && ExitToastWidget)
	{
		ExitToastWidget->ShowToast();
	}
}

void UPGAlertContainerWidget::OnExitCameraMode_EnemyToast()
{
	if (EnemyToastWidget)
	{
		EnemyToastWidget->HideBrieflyForTransition();
	}
}
