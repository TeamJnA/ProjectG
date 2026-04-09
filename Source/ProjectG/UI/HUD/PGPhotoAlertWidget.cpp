// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGPhotoAlertWidget.h"
#include "Components/Image.h"


void UPGPhotoAlertWidget::StartBlinking()
{
    if (bIsBlinking)
    {
        return;
    }

    bIsBlinking = true;

    //if (CameraIcon)
    //{
    //    CameraIcon->SetVisibility(ESlateVisibility::Visible);
    //}

    if (BlinkAnim)
    {
        PlayAnimation(BlinkAnim, 0.0f, 0);  // 0 = 무한 반복
    }
}

void UPGPhotoAlertWidget::StopBlinking()
{
    if (!bIsBlinking)
    {
        return;
    }

    bIsBlinking = false;

    if (BlinkAnim)
    {
        StopAnimation(BlinkAnim);
    }

    //if (CameraIcon)
    //{
    //    CameraIcon->SetVisibility(ESlateVisibility::Collapsed);
    //}
}