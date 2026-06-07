// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGPhotoAlertWidget.h"
#include "Components/Image.h"
#include "Character/PGPlayerCharacter.h"


void UPGPhotoAlertWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    FWidgetAnimationDynamicEvent Finished;
    Finished.BindDynamic(this, &UPGPhotoAlertWidget::HandleSlideAnimFinished);

    if (SlideInAnim)
    {
        BindToAnimationFinished(SlideInAnim, Finished);
    }

    if (SlideOutAnim)
    {
        BindToAnimationFinished(SlideOutAnim, Finished);
    }
}

void UPGPhotoAlertWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!bShown)
    {
        SnapToHiddenState();
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void UPGPhotoAlertWidget::SetPhotoAlertActive(bool bActive)
{
    if (bActive == bShown)
    {
        return;
    }
    bShown = bActive;

    if (bShown)
    {
        if (SlideOutAnim && IsAnimationPlaying(SlideOutAnim))
        {
            StopAnimation(SlideOutAnim);
        }

        if (SlideInAnim)
        {
            PlayAnimation(SlideInAnim);
        }

        if (BlinkAnim)
        {
            PlayAnimation(BlinkAnim, 0.0f, 0);
        }
    }
    else
    {
        if (SlideInAnim && IsAnimationPlaying(SlideInAnim))
        {
            StopAnimation(SlideInAnim);
        }

        if (BlinkAnim)
        {
            StopAnimation(BlinkAnim);
        }

        if (CameraIcon)
        {
            CameraIcon->SetColorAndOpacity(NormalTint);
        }

        if (SlideOutAnim)
        { 
            PlayAnimation(SlideOutAnim);
        }
    }
}

void UPGPhotoAlertWidget::HandleSlideAnimFinished()
{
    if (!bShown)
    {
        SnapToHiddenState();
    }
}

void UPGPhotoAlertWidget::SnapToHiddenState()
{
    if (AlertRoot)
    {
        AlertRoot->SetRenderTranslation(HiddenTranslation);
    }
}

void UPGPhotoAlertWidget::CollapseForCameraMode()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void UPGPhotoAlertWidget::RestoreFromCameraMode()
{
    StopAnimation(SlideInAnim);
    StopAnimation(SlideOutAnim);
    StopAnimation(BlinkAnim);

    if (CameraIcon)
    {
        CameraIcon->SetColorAndOpacity(NormalTint);
    }

    bShown = false;
    SnapToHiddenState();
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}
