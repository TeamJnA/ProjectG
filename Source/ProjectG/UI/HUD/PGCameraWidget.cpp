// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGCameraWidget.h"
#include "UI/HUD/PGAttributesWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void UPGCameraWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (BatteryFillMaterialClass && BatteryFill)
    {
        BatteryFillMID = UMaterialInstanceDynamic::Create(BatteryFillMaterialClass, this);
        BatteryFill->SetBrushFromMaterial(BatteryFillMID);
    }

    if (AttributesWidget)
    {
        AttributesWidget->BindToAttributes();
    }

    UpdateBattery(CurrentBatteryPercent);
    StartGlitchPlayback();
    UpdateZoomIndicator(0.5f);
    SetProgress(0.0f);
    ResetFocusFrame();
}

void UPGCameraWidget::NativeDestruct()
{
    StopGlitchPlayback();

    Super::NativeDestruct();
}

void UPGCameraWidget::StartGlitchPlayback()
{
    if (!GlitchOverlayImage || !GlitchMediaPlayer || !GlitchMediaSource || !GlitchMaterialClass)
    {
        return;
    }

    GlitchMID = UMaterialInstanceDynamic::Create(GlitchMaterialClass, this);
    if (GlitchMID)
    {
        GlitchOverlayImage->SetBrushFromMaterial(GlitchMID);
    }

    GlitchMediaPlayer->SetLooping(true);
    GlitchMediaPlayer->OpenSource(GlitchMediaSource);
    GlitchMediaPlayer->Play();
}

void UPGCameraWidget::StopGlitchPlayback()
{
    if (GlitchMediaPlayer)
    {
        GlitchMediaPlayer->Close();
    }
}

void UPGCameraWidget::SetProgress(float Progress)
{
    // FocusFrame 스케일 (1.0 → FocusFrameMaxScale)
    if (FocusFrame)
    {
        float Scale = FMath::Lerp(1.0f, FocusFrameMaxScale, Progress);
        FocusFrame->SetRenderScale(FVector2D(Scale, Scale));
    }
}

void UPGCameraWidget::DisplayPhotoResult(const TArray<FPhotoSubjectInfo>& Results, int32 TotalScore)
{
    UE_LOG(LogTemp, Log, TEXT("[CameraWidget] PhotoResult: Captured %d subjects, TotalScore: %d"), Results.Num(), TotalScore);
}

void UPGCameraWidget::PlayFadeInTransition(FOnCameraTransitionFinished OnFinished)
{
    CachedOnFinished = OnFinished;

    if (FadeInTransitionAnim)
    {
        FadeAnimDelegate.BindDynamic(this, &UPGCameraWidget::OnFadeTransitionFinished);
        BindToAnimationFinished(FadeInTransitionAnim, FadeAnimDelegate);
        PlayAnimation(FadeInTransitionAnim);
    }
    else
    {
        OnFadeTransitionFinished();
    }
}

void UPGCameraWidget::PlayFadeOutTransition(FOnCameraTransitionFinished OnFinished)
{
    CachedOnFinished = OnFinished;

    if (FadeOutTransitionAnim)
    {
        FadeAnimDelegate.BindDynamic(this, &UPGCameraWidget::OnFadeTransitionFinished);
        BindToAnimationFinished(FadeOutTransitionAnim, FadeAnimDelegate);
        PlayAnimation(FadeOutTransitionAnim);
    }
    else
    {
        OnFadeTransitionFinished();
    }
}

void UPGCameraWidget::OnFadeTransitionFinished()
{
    if (FadeInTransitionAnim)
    {
        UnbindFromAnimationFinished(FadeInTransitionAnim, FadeAnimDelegate);
    }
    if (FadeOutTransitionAnim)
    {
        UnbindFromAnimationFinished(FadeOutTransitionAnim, FadeAnimDelegate);
    }
    FadeAnimDelegate.Unbind();
    CachedOnFinished.ExecuteIfBound();
}

void UPGCameraWidget::UpdateZoomIndicator(float ZoomAlpha)
{
    if (!ZoomIndicator)
    {
        return;
    }

    // ZoomAlpha: 0.0(줌아웃 끝) ~ 1.0(줌인 끝)
    // 7단계(인덱스 0~6)로 스냅
    const int32 NumSteps = 6;
    int32 Step = FMath::RoundToInt(ZoomAlpha * NumSteps);
    Step = FMath::Clamp(Step, 0, NumSteps);

    float SnappedAlpha = (float)Step / (float)NumSteps;
    float Offset = FMath::Lerp(-ZoomIndicatorMaxOffset, ZoomIndicatorMaxOffset, SnappedAlpha);
    ZoomIndicator->SetRenderTranslation(FVector2D(Offset, 0.0f));
}

void UPGCameraWidget::UpdateBattery(float Percent)
{
    if (!BatteryFillMID)
    {
        return;
    }

    CurrentBatteryPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
    float DisplayValue = SnapToDisplayStep(CurrentBatteryPercent);
    BatteryFillMID->SetScalarParameterValue(FName("FillAmount"), DisplayValue);

    FLinearColor Color;
    if (DisplayValue > 0.36f)
    {
        Color = FLinearColor::White;
    }
    else if (DisplayValue > 0.11f)
    {
        Color = FLinearColor::Yellow;
    }
    else
    {
        Color = FLinearColor::Red;
    }
    BatteryFillMID->SetVectorParameterValue(FName("BatteryColor"), Color);
}

float UPGCameraWidget::SnapToDisplayStep(float Percent) const
{
    // 올림 스냅: Percent 이상인 가장 작은 스텝
    for (float Step : BatteryDisplaySteps)
    {
        if (Step >= Percent)
        {
            return Step;
        }
    }
    return BatteryDisplaySteps.Last();
}

void UPGCameraWidget::PlayCaptureEffect()
{
    // 화이트 플래시 애니메이션
    if (WhiteFlashAnim)
    {
        PlayAnimation(WhiteFlashAnim);
    }
}

void UPGCameraWidget::UpdateFocusFramePosition(const FVector2D& ScreenPosition)
{
    if (!FocusFrame)
    {
        return;
    }

    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    else
    {
        return;
    }

    FVector2D WidgetCenter = ViewportSize * 0.5f;
    FVector2D Offset = ScreenPosition - WidgetCenter;

    float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
    if (Scale > 0.0f)
    {
        Offset /= Scale;
    }

    FocusFrame->SetRenderTranslation(Offset);
}

void UPGCameraWidget::ResetFocusFrame()
{
    if (FocusFrame)
    {
        FocusFrame->SetRenderTranslation(FVector2D::ZeroVector);
        FocusFrame->SetRenderScale(FVector2D(1.0f, 1.0f));
    }
}
