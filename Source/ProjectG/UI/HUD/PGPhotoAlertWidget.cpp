// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGPhotoAlertWidget.h"
#include "Components/Image.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGCameraComponent.h"


void UPGPhotoAlertWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    const UPGCameraComponent* Cam = ResolveCameraComponent();
    if (!Cam)
    {
        return;
    }

    const bool bCanUse = Cam->HasBattery();
    if (bCanUse != bCanUseCamera)
    {
        bCanUseCamera = bCanUse;

        RefreshBlinkState();

        if (CameraIcon)
        {
            CameraIcon->SetRenderOpacity(bCanUse ? 1.0f : DisabledOpacity);
        }
    }
}

UPGCameraComponent* UPGPhotoAlertWidget::ResolveCameraComponent()
{
    if (CachedCam.IsValid())
    {
        return CachedCam.Get();
    }

    APGPlayerCharacter* Player = GetOwningPlayerPawn<APGPlayerCharacter>();
    if (!Player)
    {
        if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
        {
            Player = Cast<APGPlayerCharacter>(PC->GetPawn());
        }
    }

    if (Player)
    {
        if (UPGCameraComponent* Cam = Player->GetCameraComponent())
        {
            CachedCam = Cam;
            return Cam;
        }
    }

    return nullptr;
}

void UPGPhotoAlertWidget::StartBlinking()
{
    bBlinkRequested = true;
    RefreshBlinkState();
}

void UPGPhotoAlertWidget::StopBlinking()
{
    bBlinkRequested = false;
    RefreshBlinkState();
}

void UPGPhotoAlertWidget::RefreshBlinkState()
{
    if (!BlinkAnim)
    {
        return;
    }

    const bool bShouldPlay = bBlinkRequested && bCanUseCamera;
    if (bShouldPlay && !bAnimActive)
    {
        PlayAnimation(BlinkAnim, 0.0f, 0);
        bAnimActive = true;
    }
    else if (!bShouldPlay && bAnimActive)
    {
        StopAnimation(BlinkAnim);
        bAnimActive = false;

        if (CameraIcon)
        {
            CameraIcon->SetColorAndOpacity(NormalTint);
        }
    }
}
