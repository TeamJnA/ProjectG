// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickWindowBlood.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Interface/GimmickTargetInterface.h"
#include "Sound/PGSoundManager.h"

APGTriggerGimmickWindowBlood::APGTriggerGimmickWindowBlood()
{
    ActivationChance = 0.5f;
    bIsOneShotEvent = true;

    if (StaticMesh)
    {
        StaticMesh->SetCollisionProfileName(TEXT("BlockAll"));
        StaticMesh->SetGenerateOverlapEvents(false);
    }
}

void APGTriggerGimmickWindowBlood::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnTriggerOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void APGTriggerGimmickWindowBlood::LocalEffect(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
    float TargetSanity = 100.0f;
    IGimmickTargetInterface* GimmickTarget = Cast<IGimmickTargetInterface>(OtherActor);
    if (GimmickTarget)
    {
        TargetSanity = GimmickTarget->GetSanityValue();
    }

    const int32 MaxAdditionalEffect = MaxEffect - MinEffect;
    const float SanityNormalized = FMath::Clamp(TargetSanity, 0.0f, 100.0f);
    const float AdditionalEffectFloat = (100.0f - SanityNormalized) / 100.0f * MaxAdditionalEffect;
    const int32 AdditionalEffect = FMath::FloorToInt(AdditionalEffectFloat);
    NumEffect = MinEffect + AdditionalEffect;
    NumEffect = FMath::Clamp(NumEffect, MinEffect, FMath::Min(MaxEffect, EffectArray.Num()));

    const float NormalizedEffectCount = FMath::GetMappedRangeValueClamped(FVector2D(MinEffect, MaxEffect), FVector2D(0.0f, 1.0f), (float)NumEffect);
    TimeBetweenEffect = FMath::Lerp(MaxEffectTime, MinEffectTime, NormalizedEffectCount);

    UE_LOG(LogTemp, Log, TEXT("WindowBlood::LocalEffect: Sanity: %.1f, TotalEffect: %d, TimeBetweenEffect: %.3f"), TargetSanity, NumEffect, TimeBetweenEffect);

    StartEffect();
}

void APGTriggerGimmickWindowBlood::StartEffect()
{
    if (EffectArray.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Window::StartEffect: Error"));
        return;
    }

    ShuffleTArray(EffectArray);

    SingleEffect();
    GetWorld()->GetTimerManager().SetTimer(
        EffectTimerHandle,
        this,
        &APGTriggerGimmickWindowBlood::SingleEffect,
        TimeBetweenEffect,
        true,
        1.0f
    );
}

void APGTriggerGimmickWindowBlood::SingleEffect()
{
    if (CurrentIndex < NumEffect)
    {
        EffectArray[CurrentIndex]->SetVisibility(true, true);

        CurrentIndex++;

        if (!SoundManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("WindowBlood::SingleEffect: No valid sound manager."));
            return;
        }
        SoundManager->PlaySoundForSelf(WindowHitSoundName);
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
    }
}

template<typename T>
void APGTriggerGimmickWindowBlood::ShuffleTArray(TArray<T>& Array)
{
    int32 LastIndex = Array.Num() - 1;
    for (int32 i = 0; i <= LastIndex; ++i)
    {
        int32 RandIndex = FMath::RandRange(i, LastIndex);
        Array.Swap(i, RandIndex);
    }
}
