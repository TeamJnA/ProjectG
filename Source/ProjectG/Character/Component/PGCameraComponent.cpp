// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGCameraComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Interface/PhotographableInterface.h"
#include "UI/Manager/PGHUD.h"
#include "UI/HUD/PGCameraWidget.h"
#include "AbilitySystemComponent.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/MirrorGhost/Character/PGMirrorGhostCharacter.h"


UPGCameraComponent::UPGCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    bHandCamera = false;
}

void UPGCameraComponent::InitCameraComponent()
{
    // 저장된 배터리, 찍은 대상들 정보 복원
    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APGPlayerState* PS = Owner->GetPlayerState<APGPlayerState>())
        {
            CurrentBattery = PS->GetCameraBattery();

            for (int32 ID : PS->GetCapturedIDs())
            {
                LocalCapturedIDs.Add(ID);
            }
        }
    }
}

void UPGCameraComponent::BeginPlay()
{
    Super::BeginPlay();

    //if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    //{
    //    if (APGPlayerState* PS = Owner->GetPlayerState<APGPlayerState>())
    //    {
    //        CurrentBattery = PS->GetCameraBattery();

    //        for (int32 ID : PS->GetCapturedIDs())
    //        {
    //            LocalCapturedIDs.Add(ID);
    //        }
    //    }
    //}
}

void UPGCameraComponent::EnterCameraMode()
{
    if (bIsTransitioning || bInCameraMode)
    {
        return;
    }

    // 배터리 체크
    if (CurrentBattery <= 0.0f)
    {
        return;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC)
    {
        return;
    }

    APGHUD* HUD = Cast<APGHUD>(PC->GetHUD());
    if (!HUD)
    {
        return;
    }

    UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();
    if (!ASC || ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Player.Hand.Locked")))
    {
        return;
    }
    
    // 여기서 핸드락을 부여하지 말고 그냥 HandAction Anim ability 재생시켜서 핸드락 부여
    // 끝날때즘 애님 노티파이 -> 캐릭터의 카메라 컴포넌트 접근해서 핸드락 부여
    
    IHandItemInterface* HandInterface = Cast<IHandItemInterface>(Owner);
    if (HandInterface)
    {
        HandInterface->SetCameraMeshOnHand(true);
    }
    
    bIsTransitioning = true;

    // LensDistortion MID 할당
    LenseDistortionMID = Owner->GetLensDistortionMID();

    HUD->EnterCameraMode();

    GetWorld()->GetTimerManager().SetTimer(SettingsDelayTimerHandle, this, &UPGCameraComponent::DelayedApplySettings, 0.15f, false);

    if (TurnOnSound)
    {
        UGameplayStatics::PlaySound2D(this, TurnOnSound);
    }

    UPGCameraWidget* Widget = HUD->GetCameraWidget();
    if (Widget)
    {
        FOnCameraTransitionFinished Delegate;
        Delegate.BindDynamic(this, &UPGCameraComponent::OnEnterTransitionFinished);
        Widget->PlayFadeInTransition(Delegate);
    }
    else
    {
        OnEnterTransitionFinished();
    }
}

void UPGCameraComponent::DelayedApplySettings()
{
    ApplyCameraSettings();
}

void UPGCameraComponent::ApplyCameraSettings()
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->GetFirstPersonCamera())
    {
        return;
    }

    UCameraComponent* Camera = Owner->GetFirstPersonCamera();
    Camera->SetFieldOfView(CameraModeFOV);
    Camera->bConstrainAspectRatio = true;
    CurrentZoomFOV = CameraModeFOV;
    CurrentCameraRange = 3000.0f;

    if (LenseDistortionMID)
    {
        LenseDistortionMID->SetScalarParameterValue(FName("LenseDistortion"), 0.3f);
    }

    Camera->PostProcessSettings.BloomThreshold = -1.0f;
    Camera->PostProcessSettings.SceneFringeIntensity = 3.0f;

    Owner->SetCameraFilmGrain(0.6f);

    SetLocalGhostVisible(true);
}

void UPGCameraComponent::OnEnterTransitionFinished()
{
    bIsTransitioning = false;
    SetInCameraMode(true);
    bPhotoTaken = false;
    CameraElapsedTime = 0.0f;

    bIsTrackingTarget = false;
    TrackedTargetActor.Reset();

    GetWorld()->GetTimerManager().SetTimer(BatteryDrainTimerHandle, this, &UPGCameraComponent::DrainBattery, 0.1f, true);
    GetWorld()->GetTimerManager().SetTimer(CameraProgressTimerHandle, this, &UPGCameraComponent::UpdateCameraProgress, 0.05f, true);
}

void UPGCameraComponent::ExitCameraMode()
{
    if (bIsTransitioning || !bInCameraMode)
    {
        return;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC)
    {
        return;
    }

    APGHUD* HUD = Cast<APGHUD>(PC->GetHUD());
    if (!HUD)
    {
        return;
    }

    SetHandLockTag(false);

    // 캐릭터가 카메라 내리는 애님
    
    IHandItemInterface* HandInterface = Cast<IHandItemInterface>(Owner);
    if (HandInterface)
    {
        HandInterface->SetCameraMeshOnHand(false);
    }

    bIsTransitioning = true;
    SetInCameraMode(false);
    bIsTrackingTarget = false;
    TrackedTargetActor.Reset();
    CameraElapsedTime = 0.0f;

    GetWorld()->GetTimerManager().ClearTimer(CameraProgressTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TrackingBeepTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(SettingsDelayTimerHandle, this, &UPGCameraComponent::DelayedRestoreSettings, 0.15f, false);

    if (TurnOffSound)
    {
        UGameplayStatics::PlaySound2D(this, TurnOffSound);
    }

    UPGCameraWidget* Widget = HUD->GetCameraWidget();
    if (Widget)
    {
        FOnCameraTransitionFinished Delegate;
        Delegate.BindDynamic(this, &UPGCameraComponent::OnExitTransitionFinished);
        Widget->PlayFadeOutTransition(Delegate);
    }
    else
    {
        OnExitTransitionFinished();
    }
}

void UPGCameraComponent::DelayedRestoreSettings()
{
    RestoreCameraSettings();
}

void UPGCameraComponent::RestoreCameraSettings()
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->GetFirstPersonCamera())
    {
        return;
    }

    UCameraComponent* Camera = Owner->GetFirstPersonCamera();
    Camera->SetFieldOfView(DefaultFOV);
    Camera->bConstrainAspectRatio = false;
    CurrentZoomFOV = CameraModeFOV;
    CurrentCameraRange = 3000.0f;

    if (LenseDistortionMID)
    {
        LenseDistortionMID->SetScalarParameterValue(FName("LenseDistortion"), 0.0f);
    }

    Camera->PostProcessSettings.BloomThreshold = 0.8f;
    Camera->PostProcessSettings.SceneFringeIntensity = 1.0f;

    Owner->SetCameraFilmGrain(0.2f);

    SetLocalGhostVisible(false);
}

void UPGCameraComponent::OnExitTransitionFinished()
{
    bIsTransitioning = false;

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
    {
        if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
        {
            HUD->ExitCameraMode();
        }
    }
}

void UPGCameraComponent::ForceExitCameraMode()
{
    if (!bInCameraMode && !bIsTransitioning)
    {
        return;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    // 태그 해제
    SetHandLockTag(false);

    // 캐릭터가 카메라 내리는 애님
    
    IHandItemInterface* HandInterface = Cast<IHandItemInterface>(Owner);
    if (HandInterface)
    {
        HandInterface->SetCameraMeshOnHand(false);
    }

    // 상태 초기화
    bIsTransitioning = false;
    SetInCameraMode(false);
    bIsTrackingTarget = false;
    bPhotoTaken = false;
    TrackedTargetActor.Reset();
    CameraElapsedTime = 0.0f;

    SetLocalGhostVisible(false);

    // 타이머 전부 정리
    GetWorld()->GetTimerManager().ClearTimer(CameraProgressTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TrackingBeepTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(SettingsDelayTimerHandle);

    // 카메라 설정 즉시 복원
    RestoreCameraSettings();
}

void UPGCameraComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPGCameraComponent, bInCameraMode);
    DOREPLIFETIME(UPGCameraComponent, bHandCamera);
    DOREPLIFETIME(UPGCameraComponent, HandCameraRepCounter);
}

void UPGCameraComponent::ResetProgress()
{
    CameraElapsedTime = 0.0f;
    bIsTrackingTarget = false;
    TrackedTargetActor.Reset();
    GetWorld()->GetTimerManager().ClearTimer(TrackingBeepTimerHandle);

    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
        {
            if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
            {
                HUD->UpdateCameraProgress(0.0f);

                if (UPGCameraWidget* Widget = HUD->GetCameraWidget())
                {
                    Widget->ResetFocusFrame();
                }
            }
        }
    }
}

void UPGCameraComponent::UpdateCameraProgress()
{
    if (!bInCameraMode || bPhotoTaken)
    {
        return;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    // 현재 추적 중인 대상이 여전히 유효한지 확인
    if (bIsTrackingTarget && TrackedTargetActor.IsValid())
    {
        if (!IsTargetValid(TrackedTargetActor.Get()))
        {
            // 추적 대상이 범위 벗어남 → 추적 해제
            bIsTrackingTarget = false;
            TrackedTargetActor.Reset();
            GetWorld()->GetTimerManager().ClearTimer(TrackingBeepTimerHandle);

            if (CameraElapsedTime > 0.0f)
            {
                ResetProgress();
            }
            return;
        }
    }

    // 추적 중이 아니면 새 대상 탐색
    if (!bIsTrackingTarget)
    {
        AActor* NewTarget = FindClosestSubjectActor();
        if (!NewTarget)
        {
            if (CameraElapsedTime > 0.0f)
            {
                ResetProgress();
            }
            return;
        }

        bIsTrackingTarget = true;
        TrackedTargetActor = NewTarget;

        if (TargetFoundSound)
        {
            UGameplayStatics::PlaySound2D(this, TargetFoundSound);
        }

        GetWorld()->GetTimerManager().SetTimer(TrackingBeepTimerHandle, this, &UPGCameraComponent::PlayTrackingSound, 0.9f, true);
    }

    // 새로운 피사체가 있으면 Progress 진행
    CameraElapsedTime += 0.05f;
    float Progress = FMath::Clamp(CameraElapsedTime / PhotoDuration, 0.0f, 1.0f);

    if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
    {
        if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
        {
            HUD->UpdateCameraProgress(Progress);

            if (UPGCameraWidget* Widget = HUD->GetCameraWidget())
            {
                // FocusFrame 스크린 위치 업데이트
                if (TrackedTargetActor.IsValid())
                {
                    IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(TrackedTargetActor.Get());
                    if (Photographable)
                    {
                        FVector2D ScreenPos;
                        if (PC->ProjectWorldLocationToScreen(Photographable->GetPhotoTargetLocation(), ScreenPos))
                        {
                            Widget->UpdateFocusFramePosition(ScreenPos);
                        }
                    }
                }
            }
        }
    }

    // 촬영 완료
    if (CameraElapsedTime >= PhotoDuration)
    {
        GetWorld()->GetTimerManager().ClearTimer(CameraProgressTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(TrackingBeepTimerHandle);
        bIsTrackingTarget = false;
        bPhotoTaken = true;

        // 추적 대상만 촬영
        if (TrackedTargetActor.IsValid())
        {
            IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(TrackedTargetActor.Get());
            if (Photographable)
            {
                FPhotoSubjectInfo Info = Photographable->GetPhotoSubjectInfo();
                Server_TakePhoto(Info.SubjectID);
            }

            // 셔터 사운드
            if (ShutterSound)
            {
                UGameplayStatics::PlaySound2D(this, ShutterSound);
            }

            Owner->FireCameraFlash();

            if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
            {
                if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
                {
                    if (UPGCameraWidget* Widget = HUD->GetCameraWidget())
                    {
                        Widget->PlayCaptureEffect();
                    }
                }
            }
        }
    }
}

void UPGCameraComponent::OnRep_HandCamera()
{
    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        Owner->AttachItemCameraOnHand(bHandCamera);
    }
}

void UPGCameraComponent::Server_SetHandCameraMesh_Implementation(bool bInHand)
{
    bHandCamera = bInHand;
    HandCameraRepCounter++;

    UE_LOG(LogTemp, Log, TEXT("Server_SetHandCameraMesh : HasAuthority ? %d"), GetOwner()->HasAuthority());
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("Server_SetHandCameraMesh : HasAuthority true"));
        OnRep_HandCamera();
    }
}

AActor* UPGCameraComponent::FindClosestSubjectActor() const
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->GetFirstPersonCamera())
    {
        return nullptr;
    }

    UCameraComponent* Camera = Owner->GetFirstPersonCamera();
    const FVector CameraLocation = Camera->GetComponentLocation();
    const FVector ForwardVector = Camera->GetForwardVector();
    const float HalfFOVRad = FMath::DegreesToRadians(CurrentZoomFOV * 0.6f * 0.5f);
    const float CosHalfFOV = FMath::Cos(HalfFOVRad);

    AActor* ClosestActor = nullptr;
    float ClosestDist = MAX_FLT;

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor == Owner)
        {
            continue;
        }

        IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(Actor);
        if (!Photographable || !Photographable->IsPhotographable())
        {
            continue;
        }

        FPhotoSubjectInfo Info = Photographable->GetPhotoSubjectInfo();
        if (LocalCapturedIDs.Contains(Info.SubjectID))
        {
            continue;
        }

        FVector TargetLocation = Photographable->GetPhotoTargetLocation();
        FVector DirectionToActor = TargetLocation - CameraLocation;
        float Distance = DirectionToActor.Size();
        float MaxRange = FMath::Min(CurrentCameraRange, Photographable->GetPhotoDetectionRange());
        if (Distance > MaxRange)
        {
            continue;
        }

        DirectionToActor.Normalize();
        if (FVector::DotProduct(ForwardVector, DirectionToActor) < CosHalfFOV)
        {
            continue;
        }

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Owner);
        Params.AddIgnoredActor(Actor);

        bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TargetLocation, ECC_GameTraceChannel7, Params);
        if (bBlocked)
        {
            continue;
        }

        if (Distance < ClosestDist)
        {
            ClosestDist = Distance;
            ClosestActor = Actor;
        }
    }

    return ClosestActor;
}

bool UPGCameraComponent::IsTargetValid(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->GetFirstPersonCamera())
    {
        return false;
    }

    IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(Target);
    if (!Photographable || !Photographable->IsPhotographable())
    {
        return false;
    }

    FPhotoSubjectInfo Info = Photographable->GetPhotoSubjectInfo();
    if (LocalCapturedIDs.Contains(Info.SubjectID))
    {
        return false;
    }

    UCameraComponent* Camera = Owner->GetFirstPersonCamera();
    const FVector CameraLocation = Camera->GetComponentLocation();

    FVector TargetLocation = Photographable->GetPhotoTargetLocation();
    FVector DirectionToActor = TargetLocation - CameraLocation;
    float Distance = DirectionToActor.Size();
    float MaxRange = FMath::Min(CurrentCameraRange, Photographable->GetPhotoDetectionRange());
    if (Distance > MaxRange)
    {
        return false;
    }

    DirectionToActor.Normalize();
    const float HalfFOVRad = FMath::DegreesToRadians(CurrentZoomFOV * 0.6f * 0.5f);
    if (FVector::DotProduct(Camera->GetForwardVector(), DirectionToActor) < FMath::Cos(HalfFOVRad))
    {
        return false;
    }

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    Params.AddIgnoredActor(Target);

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TargetLocation, ECC_GameTraceChannel7, Params);
    return !bBlocked;
}

void UPGCameraComponent::Server_TakePhoto_Implementation(int32 SubjectID)
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    // 추적 대상만 촬영
    TArray<FPhotoSubjectInfo> Results;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(*It);
        if (Photographable)
        {
            FPhotoSubjectInfo Info = Photographable->GetPhotoSubjectInfo();
            if (Info.SubjectID == SubjectID)
            {
                Results.Add(Info);
                break;
            }
        }
    }

    APGPlayerState* PS = Owner->GetPlayerState<APGPlayerState>();
    if (PS)
    {
        PS->AddPhotoResult(Results);
        Client_PhotoResult(Results, PS->GetPhotoScore());
    }
}

void UPGCameraComponent::Client_PhotoResult_Implementation(const TArray<FPhotoSubjectInfo>& Results, int32 NewTotalScore)
{
    // 로컬 중복 캐시 업데이트
    for (const FPhotoSubjectInfo& Result : Results)
    {
        LocalCapturedIDs.Add(Result.SubjectID);
    }

    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
        {
            if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
            {
                HUD->DisplayPhotoResult(Results, NewTotalScore);
            }
        }
    }

    // 촬영 완료 후 리셋, 다시 촬영 가능
    bPhotoTaken = false;
    ResetProgress();

    if (bInCameraMode)
    {
        GetWorld()->GetTimerManager().SetTimer(CameraProgressTimerHandle, this, &UPGCameraComponent::UpdateCameraProgress, 0.05f, true);
    }
}

void UPGCameraComponent::AdjustZoom(float AxisValue)
{
    if (!bInCameraMode)
    {
        return;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    float PrevFOV = CurrentZoomFOV;

    // 휠 위로(양수) = Zoom In = FOV 감소
    CurrentZoomFOV -= AxisValue * ZoomStep;
    CurrentZoomFOV = FMath::Clamp(CurrentZoomFOV, MinZoomFOV, MaxZoomFOV);

    if (Owner->GetFirstPersonCamera())
    {
        Owner->GetFirstPersonCamera()->SetFieldOfView(CurrentZoomFOV);
    }

    // FOV가 실제로 변했을 때만 사운드
    if (CurrentZoomFOV != PrevFOV)
    {
        USoundBase* Sound = (AxisValue > 0) ? ZoomInSound : ZoomOutSound;
        if (Sound)
        {
            UGameplayStatics::PlaySound2D(this, Sound);
        }
    }

    float Alpha = 1.0f - (CurrentZoomFOV - MinZoomFOV) / (MaxZoomFOV - MinZoomFOV);
    CurrentCameraRange = FMath::Lerp(MinCameraRange, MaxCameraRange, Alpha);

    // Zoom Indicator 업데이트
    if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
    {
        if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
        {
            if (UPGCameraWidget* Widget = HUD->GetCameraWidget())
            {
                // MaxZoomFOV(줌아웃) → 0.0, MinZoomFOV(줌인) → 1.0
                Widget->UpdateZoomIndicator(Alpha);
            }
        }
    }
}

void UPGCameraComponent::CameraHandAnimFinished()
{
    if (bInCameraMode)
    {
        UE_LOG(LogTemp, Log, TEXT("CameraHandAnimFinished %d"), GetOwner()->HasAuthority());
        SetHandLockTag(true);
    }
}

void UPGCameraComponent::DrainBattery()
{
    CurrentBattery -= BatteryDrainRate * 0.1f;  // 0.1초 간격
    CurrentBattery = FMath::Max(CurrentBattery, 0.0f);

    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APGPlayerState* PS = Owner->GetPlayerState<APGPlayerState>())
        {
            PS->SetCameraBattery(CurrentBattery);
        }
    }

    // 위젯 업데이트
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (Owner)
    {
        if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
        {
            if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
            {
                if (UPGCameraWidget* Widget = HUD->GetCameraWidget())
                {
                    Widget->UpdateBattery(CurrentBattery);
                }
            }
        }
    }

    if (CurrentBattery <= 0.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimerHandle);
        ExitCameraMode();
    }
}

void UPGCameraComponent::AddBattery(float Amount)
{
    CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBattery);

    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APGPlayerState* PS = Owner->GetPlayerState<APGPlayerState>())
        {
            PS->SetCameraBattery(CurrentBattery);
        }
    }
}

void UPGCameraComponent::PlayTrackingSound()
{
    if (TrackingBeepSound && bIsTrackingTarget)
    {
        UGameplayStatics::PlaySound2D(this, TrackingBeepSound);
    }
}

void UPGCameraComponent::SetHandLockTag(bool bHandLock)
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }
    
    UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag("Player.Hand.Locked"));

    if (bHandLock)
    {
        // ASC->AddLooseGameplayTags(TagContainer);
        Owner->AddTagToCharacter(1, TagContainer);
    }
    else
    {
        // ASC->RemoveLooseGameplayTags(TagContainer);
        Owner->RemoveTagFromCharacter(1, TagContainer);
    }
}

void UPGCameraComponent::SetInCameraMode(bool bNewMode)
{
    // 로컬 클라이언트에서 즉시 변경 (예측)
    bInCameraMode = bNewMode;

    // 서버에 보고하여 다른 클라이언트들에게 전파하도록 함
    AActor* Owner = GetOwner();
    if (Owner && !Owner->HasAuthority())
    {
        Server_SetInCameraMode(bNewMode);
    }
}

void UPGCameraComponent::Server_SetInCameraMode_Implementation(bool bNewMode)
{
    bInCameraMode = bNewMode;
}

void UPGCameraComponent::SetLocalGhostVisible(bool bVisible)
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC || !PC->PlayerState)
    {
        return;
    }

    for (TActorIterator<APGGhostCharacter> It(GetWorld()); It; ++It)
    {
        APGGhostCharacter* Ghost = *It;
        if (Ghost && Ghost->GetTargetPlayerState() == PC->PlayerState)
        {
            Ghost->SetCameraModeVisible(bVisible);
            break;
        }
    }

    for (TActorIterator<APGMirrorGhostCharacter> It(GetWorld()); It; ++It)
    {
        APGMirrorGhostCharacter* MirrorGhost = *It;
        if (MirrorGhost && MirrorGhost->GetTargetPlayer() == Owner)
        {
            MirrorGhost->SetCameraModeVisible(bVisible);
            break;
        }
    }
}
