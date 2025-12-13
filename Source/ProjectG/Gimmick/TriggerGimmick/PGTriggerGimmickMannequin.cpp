// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickMannequin.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PGPlayerCharacter.h"
#include "Sound/PGSoundManager.h"
#include "Interface/GimmickTargetInterface.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"


APGTriggerGimmickMannequin::APGTriggerGimmickMannequin()
{
    MannequinBody = StaticMesh;
    MannequinBody->SetCollisionProfileName(TEXT("BlockAll"));

    MannequinHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MannequinHead"));
    MannequinHead->SetupAttachment(MannequinBody, FName("MannequinNeckSocket"));
    MannequinHead->SetCollisionProfileName(TEXT("BlockAll"));
    MannequinHead->SetGenerateOverlapEvents(false);

    EffectRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EffectRangeSphere"));
    EffectRangeSphere->SetupAttachment(RootComponent);
    EffectRangeSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    EffectRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EffectRangeSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    EffectRangeSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    EffectRangeSphere->SetGenerateOverlapEvents(true);
}

void APGTriggerGimmickMannequin::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsTracking)
    {
        return;
    }

    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    if (UKismetMathLibrary::RandomFloat() > ActivationChance)
    {
        return;
    }

    Multicast_PlayLocalEffect(OtherActor, OtherComp);
}

void APGTriggerGimmickMannequin::LocalEffect(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
    if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor))
    {
        if (!bIsTracking)
        {
            UE_LOG(LogTemp, Log, TEXT("Mannequin::OnLocalTriggerOverlap: Overlapped on local."));

            StartTracking(Player);
        }
    }
}

void APGTriggerGimmickMannequin::StartTracking(APGPlayerCharacter* Player)
{
    if (bIsTracking || !Player)
    {
        UE_LOG(LogTemp, Error, TEXT("Mannequin::StartTracking: Error"));
        return;
    }

    if (!SoundManager.IsValid())
    {
        SoundManager = Cast<APGSoundManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APGSoundManager::StaticClass()));
        if (!SoundManager.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Mannequin::StartTracking: No valid sound manager"));
        }
    }

    TargetPlayer = Player;
    bIsTracking = true;
    SoundCount = 0;
    CurrentSanityDamageApplied = 0.0f;
    if (EffectRangeSphere)
    {
        EffectRangeSphere->OnComponentEndOverlap.AddDynamic(this, &APGTriggerGimmickMannequin::OnEffectRangeOverlapEnd);
    }

    GetWorldTimerManager().SetTimer(
        TrackingTimerHandle,
        this,
        &APGTriggerGimmickMannequin::UpdateTracking,
        UpdateRate,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Mannequin Tracking Started for %s."), *Player->GetName());
}

void APGTriggerGimmickMannequin::StopTracking()
{
    if (!bIsTracking)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(TrackingTimerHandle);

    if (EffectRangeSphere)
    {
        EffectRangeSphere->OnComponentEndOverlap.RemoveDynamic(this, &APGTriggerGimmickMannequin::OnEffectRangeOverlapEnd);
    }   
    TargetPlayer = nullptr;
    bIsTracking = false;
    SoundCount = 0;
    CurrentSanityDamageApplied = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Mannequin Tracking Stopped"));
}

void APGTriggerGimmickMannequin::UpdateTracking()
{
    APGPlayerCharacter* Player = TargetPlayer.Get();
    if (!Player)
    {
        StopTracking();
        return;
    }

    if (IsPlayerLookingAtMannequin(Player))
    {
        StopTracking();
        return;
    }

    UpdateHeadRotation(Player);
    ApplyContinuousEffect(Player);
    PlaySoundLocal();
}

void APGTriggerGimmickMannequin::UpdateHeadRotation(APGPlayerCharacter* Player)
{
    const FVector PlayerLocation = Player->GetActorLocation();
    const FVector MannequinLocation = MannequinHead->GetComponentLocation();
    const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(MannequinLocation, PlayerLocation);

    const FRotator WorldHeadRotation = FRotator(0.0f, LookAtRotation.Yaw, 0.0f);
    const FRotator TargetRelativeRotation = WorldHeadRotation - GetActorRotation();
    const FRotator CurrentRelativeRotation = MannequinHead->GetRelativeRotation();
    const float DeltaTime = UpdateRate;
    const FRotator NewRotation = FMath::RInterpTo(CurrentRelativeRotation, TargetRelativeRotation, DeltaTime, RotationSpeed);

    const FRotator TargetHeadRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);
    MannequinHead->SetRelativeRotation(TargetHeadRotation);
}

void APGTriggerGimmickMannequin::ApplyContinuousEffect(APGPlayerCharacter* Player)
{
    if (CurrentSanityDamageApplied >= MaxSanityDamage)
    {
        return;
    }

    IGimmickTargetInterface* GimmickInterface = Cast<IGimmickTargetInterface>(Player);
    if (GimmickInterface)
    {
        GimmickInterface->RequestApplyGimmickEffect(TriggerEffectClass);
    }

    CurrentSanityDamageApplied += 0.1f;
    CurrentSanityDamageApplied = FMath::Min(CurrentSanityDamageApplied, MaxSanityDamage);
}

void APGTriggerGimmickMannequin::PlaySoundLocal()
{
    APGSoundManager* SM = SoundManager.Get();
    if (!SM)
    {
        return;
    }

    if (SoundCount >= 5)
    {
        SM->PlaySoundForSelf(MannequinSoundName);
        SoundCount = 0;
    }
    else
    {
        SoundCount++;
    }
}

bool APGTriggerGimmickMannequin::IsPlayerLookingAtMannequin(APGPlayerCharacter* Player) const
{
    if (!Player->GetFirstPersonCamera())
    {
        return true;
    }

    const FVector CameraLocation = Player->GetFirstPersonCamera()->GetComponentLocation();
    const FVector CameraForwardVector = Player->GetFirstPersonCamera()->GetForwardVector();

    const FVector MannequinHeadLocation = MannequinHead->GetComponentLocation();
    const FVector DirectionToMannequin = (MannequinHeadLocation - CameraLocation).GetSafeNormal();

    const float DotProduct = FVector::DotProduct(CameraForwardVector, DirectionToMannequin);
    if (DotProduct < LookAngleThreshold)
    {
        return false;
    }

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(Player);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        MannequinHeadLocation,
        ECC_Visibility,
        Params
    );

    return !bHit || (HitResult.GetActor() == this);
}

void APGTriggerGimmickMannequin::OnEffectRangeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bIsTracking || !TargetPlayer.IsValid() || OtherActor != TargetPlayer.Get())
    {
        UE_LOG(LogTemp, Error, TEXT("Mannequin::OnEffectRangeOverlapEnd: [%s]."), *OtherActor->GetName());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Mannequin::OnEffectRangeOverlapEnd: TargetPlayer '%s' left the effect range. Stopping tracking."), *OtherActor->GetName());
    StopTracking();
}
