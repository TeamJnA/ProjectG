// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/PGPhotoTypes.h"
#include "PGCameraComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraProgressUpdate, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhotoTaken);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API UPGCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UPGCameraComponent();

    void InitCameraComponent();

    void EnterCameraMode();
    void ExitCameraMode();
    void ForceExitCameraMode();

    FORCEINLINE bool IsInCameraMode() const { return bInCameraMode; }
    FORCEINLINE bool IsTransitioning() const { return bIsTransitioning; }
    FORCEINLINE bool IsAlreadyCaptured(int32 SubjectID) const { return LocalCapturedIDs.Contains(SubjectID); }

    FORCEINLINE bool HasBattery() const { return CurrentBattery > 0.0f; }
    FORCEINLINE float GetBatteryPercent() const { return CurrentBattery; }
    void AddBattery(float Amount);

    UFUNCTION(Server, Reliable)
    void Server_TakePhoto(int32 SubjectID);

    UFUNCTION(Client, Reliable)
    void Client_PhotoResult(const TArray<FPhotoSubjectInfo>& Results, int32 NewTotalScore);

    void AdjustZoom(float AxisValue);

    FOnCameraProgressUpdate OnCameraProgressUpdate;
    FOnPhotoTaken OnPhotoTaken;

protected:
    virtual void BeginPlay() override;

    void UpdateCameraProgress();
    void ResetProgress();

    UFUNCTION()
    void OnEnterTransitionFinished();

    UFUNCTION()
    void DelayedApplySettings();

    void ApplyCameraSettings();

    UFUNCTION()
    void OnExitTransitionFinished();

    UFUNCTION()
    void DelayedRestoreSettings();

    void RestoreCameraSettings();

    void DrainBattery();

    void PlayTrackingSound();

    AActor* FindClosestSubjectActor() const;
    bool IsTargetValid(AActor* Target) const;

    void SetLocalGhostVisible(bool bVisible);

    // 클라이언트 로컬 중복 체크용
    TSet<int32> LocalCapturedIDs;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> LenseDistortionMID;

    // On/Off
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> TurnOnSound;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> TurnOffSound;

    // 줌 사운드
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> ZoomInSound;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> ZoomOutSound;

    // 타겟 발견 사운드
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> TargetFoundSound;

    // 타겟 추적 사운드
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> TrackingBeepSound;

    // 셔터 사운드
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> ShutterSound;

    UPROPERTY()
    TWeakObjectPtr<AActor> TrackedTargetActor;

    FTimerHandle CameraProgressTimerHandle;
    FTimerHandle SettingsDelayTimerHandle;
    FTimerHandle BatteryDrainTimerHandle;
    FTimerHandle TrackingBeepTimerHandle;

    float CameraElapsedTime = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float PhotoDuration = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float MaxCameraRange = 4000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float MinCameraRange = 2000.0f;

    float CurrentCameraRange = 3000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DefaultFOV = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraModeFOV = 75.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float MaxZoomFOV = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float MinZoomFOV = 60.0f;

    float CurrentZoomFOV = 75.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float ZoomStep = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Battery")
    float MaxBattery = 1.0f;

    float CurrentBattery = 1.0f;  // 0.0 ~ 1.0

    UPROPERTY(EditDefaultsOnly, Category = "Battery")
    float BatteryDrainRate = 0.01f;  // 초당 소모량

    bool bInCameraMode = false;
    bool bPhotoTaken = false;

    // 타겟 추적 상태
    bool bIsTrackingTarget = false;
    bool bIsTransitioning = false;
};
