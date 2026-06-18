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
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/MirrorGhost/Character/PGMirrorGhostCharacter.h"
#include "Utils/PGPhotoSubjectRegistry.h"


UPGCameraComponent::UPGCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    bHandCamera = false;

    CameraTransform = FTransform(
        FRotator(-64.481403f, 120.417069f, 102.291763f),  // Rotation
        FVector(-5.7f, 11.68f, -2.85f),                   // Location
        FVector(0.35f, 0.35f, 0.35f)                      // Scale
    );
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

    // 배터리 체크
    if (!HasBattery())
    {
        //TODO : 경고음과 함께 카메라 안 켜지도록
        return;
    }
    
    // 여기서 핸드락을 부여하지 말고 그냥 HandAction Anim ability 재생시켜서 핸드락 부여
    // 끝날때즘 애님 노티파이 -> 캐릭터의 카메라 컴포넌트 접근해서 핸드락 부여
    Owner->SetCameraMeshOnHand(true);
    
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
    bCaptureLocked = false;
    bHasFramedSubject = false;
    FocusIndicatorAlpha = 0.0f;
    SetInCameraMode(true);

    GetWorld()->GetTimerManager().SetTimer(BatteryDrainTimerHandle, this, &UPGCameraComponent::DrainBattery, 0.1f, true);
    GetWorld()->GetTimerManager().SetTimer(FocusIndicatorTimerHandle, this, &UPGCameraComponent::UpdateFocusIndicator, 0.05f, true);
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

    UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();
    if (!ASC || !ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Gameplay.State.CameraOn")))
    {
        return;
    }

    // Remove Handlock
    Owner->Server_SetHandLockByGameplayEffect(false);

    // 캐릭터가 카메라 내리는 애님
    Owner->SetCameraMeshOnHand(false);

    HUD->BeginExitCameraTransition();

    bIsTransitioning = true;
    bCaptureLocked = false;
    bHasFramedSubject = false;
    FocusIndicatorAlpha = 0.0f;
    SetInCameraMode(false);

    GetWorld()->GetTimerManager().ClearTimer(CaptureLockTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(FocusIndicatorTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BeepTimerHandle);
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
    Owner->Server_SetHandLockByGameplayEffect(false);

    // 캐릭터가 카메라 내리는 애님
    Owner->SetCameraMeshOnHand(false);

    // 상태 초기화
    bIsTransitioning = false;
    bCaptureLocked = false;
    bHasFramedSubject = false;
    FocusIndicatorAlpha = 0.0f;
    SetInCameraMode(false);

    SetLocalGhostVisible(false);

    // 타이머 전부 정리
    GetWorld()->GetTimerManager().ClearTimer(SettingsDelayTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CaptureLockTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(FocusIndicatorTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BeepTimerHandle);

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

void UPGCameraComponent::TryCapture()
{
    if (!bInCameraMode || bIsTransitioning || bCaptureLocked)
    {
        return;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    TArray<AActor*> Subjects;
    GatherFramedSubjects(Subjects);

    bCaptureLocked = true;
    GetWorld()->GetTimerManager().SetTimer(CaptureLockTimerHandle, [this]() { bCaptureLocked = false; }, CaptureLockDuration, false);

    // 1) 셔터 사운드
    if (ShutterSound)
    {
        UGameplayStatics::PlaySound2D(this, ShutterSound);
    }

    // 2) 플래쉬 전 캡처
    UTextureRenderTarget2D* RT = Owner->CapturePhoto();
    if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
    {
        if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
        {
            HUD->BeginCaptureSequence(RT);
            HUD->StartCaptureCooldown(CaptureLockDuration);
        }
    }

    // 3) 카메라 플래쉬
    Owner->FireCameraFlash();

    Server_TakePhoto(Subjects);
}

void UPGCameraComponent::Server_TakePhoto_Implementation(const TArray<AActor*>& Subjects)
{
    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner)
    {
        return;
    }

    // 촬영 가능 거리 내 모든 subject 촬영처리
    TArray<FPhotoSubjectInfo> Captured;
    for (AActor* Actor : Subjects)
    {
        if (IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(Actor))
        {
            Captured.Add(Photographable->GetPhotoSubjectInfo());
        }
    }

    APGPlayerState* PS = Owner->GetPlayerState<APGPlayerState>();
    if (PS)
    {
        const TArray<FPhotoCaptureResult> Entries = PS->AddPhotoResult(Captured);
        Client_PhotoResult(Entries);
    }
}

void UPGCameraComponent::Client_PhotoResult_Implementation(const TArray<FPhotoCaptureResult>& Entries)
{
    // 처음 찍는 대상인지(Enemy는 행동 무관하게 종 자체를 처음 찍는지 체크)
    TArray<int32> NewlyRecordedKeys;
    for (const FPhotoCaptureResult& Entry : Entries)
    {
        const bool bSpeciesNew = PhotoID::IsMonster(Entry.SubjectID) ? Entry.bNewSpecies : Entry.bNewRecord;
        if (bSpeciesNew)
        {
            NewlyRecordedKeys.AddUnique(PhotoID::GetSpeciesKey(Entry.SubjectID));
        }
    }

    // 로컬 중복 캐시 업데이트
    for (const FPhotoCaptureResult& Entry : Entries)
    {
        LocalCapturedIDs.Add(Entry.SubjectID);
    }

    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
        {
            if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
            {
                HUD->DisplayCaptureLog(Entries);
                HUD->NotifyNewlyCapturedSpeciesKeys(NewlyRecordedKeys);
            }
        }
    }
}

void UPGCameraComponent::AddToLocalCapturedIDs(int32 SubjectID)
{
    LocalCapturedIDs.Add(SubjectID);
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

void UPGCameraComponent::UpdateFocusIndicator()
{
    if (!bInCameraMode)
    {
        return;
    }

    const bool bPresent = HasValidUncapturedSubjectInFrame();
    if (bPresent != bHasFramedSubject)
    {
        bHasFramedSubject = bPresent;
        if (bPresent)
        {
            if (TargetFoundSound)
            {
                UGameplayStatics::PlaySound2D(this, TargetFoundSound);
            }
            GetWorld()->GetTimerManager().SetTimer(BeepTimerHandle, this, &UPGCameraComponent::PlayBeep, FocuseBeepInterval, true);
        }
        else
        {
            GetWorld()->GetTimerManager().ClearTimer(BeepTimerHandle);
        }
    }

    const float Target = bPresent ? 1.0f : 0.0f;
    FocusIndicatorAlpha = FMath::FInterpTo(FocusIndicatorAlpha, Target, 0.05f, FocusExpandSpeed);
    if (APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
        {
            if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
            {
                HUD->UpdateCameraProgress(FocusIndicatorAlpha);
            }
        }
    }
}

void UPGCameraComponent::PlayBeep()
{
    if (TrackingBeepSound)
    {
        UGameplayStatics::PlaySound2D(this, TrackingBeepSound);
    }
}

bool UPGCameraComponent::IsSubjectFramed(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    APGPlayerCharacter* Owner = Cast<APGPlayerCharacter>(GetOwner());
    if (!Owner || !Owner->GetFirstPersonCamera())
    {
        return false;
    }

    IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(Actor);
    if (!Photographable || !Photographable->IsPhotographable())
    {
        return false;
    }

    UCameraComponent* Camera = Owner->GetFirstPersonCamera();
    const FVector CameraLocation = Camera->GetComponentLocation();
    const FVector TargetLocation = Photographable->GetPhotoTargetLocation();

    FVector DirectionToActor = TargetLocation - CameraLocation;
    const float Distance = DirectionToActor.Size();
    const float MaxRange = FMath::Min(CurrentCameraRange, Photographable->GetPhotoDetectionRange());
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
    Params.AddIgnoredActor(Actor);
    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TargetLocation, ECC_GameTraceChannel7, Params);

    return !bBlocked;
}

bool UPGCameraComponent::HasValidUncapturedSubjectInFrame() const
{
    const UPGPhotoSubjectRegistry* Registry = GetWorld() ? GetWorld()->GetSubsystem<UPGPhotoSubjectRegistry>() : nullptr;
    if (!Registry)
    {
        return false;
    }

    for (const TWeakObjectPtr<AActor>& Weak : Registry->GetSubjects())
    {
        AActor* Actor = Weak.Get();
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }

        if (!IsSubjectFramed(Actor))
        {
            continue;
        }

        IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(Actor);
        if (LocalCapturedIDs.Contains(Photographable->GetPhotoSubjectInfo().SubjectID))
        {
            continue;
        }

        return true;
    }

    return false;
}

void UPGCameraComponent::GatherFramedSubjects(TArray<AActor*>& Out) const
{
    Out.Reset();
    const UPGPhotoSubjectRegistry* Registry = GetWorld() ? GetWorld()->GetSubsystem<UPGPhotoSubjectRegistry>() : nullptr;
    if (!Registry)
    {
        return;
    }

    TSet<int32> SeenIDs;
    for (const TWeakObjectPtr<AActor>& Weak : Registry->GetSubjects())
    {
        AActor* Actor = Weak.Get();
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }

        if (!IsSubjectFramed(Actor))
        {
            continue;
        }

        IPhotographableInterface* Photographable = Cast<IPhotographableInterface>(Actor);
        const int32 ID = Photographable->GetPhotoSubjectInfo().SubjectID;
        if (SeenIDs.Contains(ID))
        {
            continue;
        }

        SeenIDs.Add(ID);
        Out.Add(Actor);
    }
}
