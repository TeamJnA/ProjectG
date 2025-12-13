// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "GameplayEffect.h"
#include "PGTriggerGimmickMannequin.generated.h"

class APGPlayerCharacter;
class UAbilitySystemComponent;
class USphereComponent;
class APGSoundManager;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGTriggerGimmickMannequin : public APGTriggerGimmickBase
{
	GENERATED_BODY()

public:
    APGTriggerGimmickMannequin();

protected:
    virtual void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, 
        bool bFromSweep, 
        const FHitResult& SweepResult) override;
    virtual void LocalEffect(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

    void StartTracking(APGPlayerCharacter* Player);
    void StopTracking();
    void UpdateTracking();
    void UpdateHeadRotation(APGPlayerCharacter* Player);
    void ApplyContinuousEffect(APGPlayerCharacter* Player);
    void PlaySoundLocal();
    bool IsPlayerLookingAtMannequin(APGPlayerCharacter* Player) const;

    UFUNCTION()
    void OnEffectRangeOverlapEnd(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mannequin Config")
    FName MannequinSoundName = FName("SFX_Mannequin");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mannequin Components")
    TObjectPtr<UStaticMeshComponent> MannequinBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mannequin Components")
    TObjectPtr<UStaticMeshComponent> MannequinHead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mannequin Components")
    TObjectPtr<USphereComponent> EffectRangeSphere;

    UPROPERTY()
    TWeakObjectPtr<APGPlayerCharacter> TargetPlayer;

    UPROPERTY()
    TWeakObjectPtr<APGSoundManager> SoundManager;

    FTimerHandle TrackingTimerHandle;

    int32 SoundCount = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Mannequin Config")
    float RotationSpeed = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Mannequin Config")
    float UpdateRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Mannequin Config")
    float LookAngleThreshold = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mannequin Config")
    float MaxSanityDamage = 5.0f;

    UPROPERTY()
    float CurrentSanityDamageApplied = 0.0f;

    bool bIsTracking = false;
};
