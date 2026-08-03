// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/Ability/PGBlindAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectG/AbilitySystem/PGAbilitySystemComponent.h"

#include "PGLogChannels.h"


APGBlindCharacter::APGBlindCharacter()
{
    BlindAttributeSet = CreateDefaultSubobject<UPGBlindAttributeSet>("BlindAttributeSet");

    // 숨소리: 3D
    BreathAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathAudio"));
    BreathAudioComponent->SetupAttachment(RootComponent);
    BreathAudioComponent->bAutoActivate = false;
    BreathAudioComponent->bOverrideAttenuation = true;
    BreathAudioComponent->AttenuationOverrides.bAttenuate = true;
    BreathAudioComponent->AttenuationOverrides.FalloffDistance = 3200.0f;

    // Chase 사운드: 감쇠 없음, 전역
    ChaseAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ChaseAudio"));
    ChaseAudioComponent->SetupAttachment(RootComponent);
    ChaseAudioComponent->bAutoActivate = false;
    ChaseAudioComponent->bOverrideAttenuation = true;
    ChaseAudioComponent->AttenuationOverrides.bAttenuate = false;

    // Chase 완료 후 사운드: 3D
    GrowlAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("GrowlAudio"));
    GrowlAudioComponent->SetupAttachment(RootComponent);
    GrowlAudioComponent->bAutoActivate = false;
    GrowlAudioComponent->bOverrideAttenuation = true;
    GrowlAudioComponent->AttenuationOverrides.bAttenuate = true;
    GrowlAudioComponent->AttenuationOverrides.FalloffDistance = 3200.0f;

    CameraShakeSource = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("CameraShakeSource"));
    CameraShakeSource->SetupAttachment(RootComponent);
    CameraShakeSource->InnerAttenuationRadius = 1000.0f;
    CameraShakeSource->OuterAttenuationRadius = 2200.0f;
}

FPhotoSubjectInfo APGBlindCharacter::GetPhotoSubjectInfo() const
{
    int32 ID = 0;
    int32 Score = 0;

    if (SoundState == EBlindSoundState::Attacking)
    {
        ID = PhotoID::Blind_Attacking;
        Score = 5;
    }
    else
    {
        switch (HuntLevel)
        {
            case EBlindHuntLevel::Exploration:
            {
                ID = PhotoID::Blind_Exploring;
                Score = 10;
                break;
            }

            case EBlindHuntLevel::Investigation:
            case EBlindHuntLevel::Chase:
            {
                ID = PhotoID::Blind_Chasing;
                Score = 10;
                break;
            }
        }
    }

    return FPhotoSubjectInfo(ID, Score);
}

FVector APGBlindCharacter::GetPhotoTargetLocation() const
{
    return GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
}

void APGBlindCharacter::SetHuntLevel(EBlindHuntLevel newHuntLevel)
{
    checkf(HasAuthority(), TEXT("서버에서만 호출되어야 합니다."));

    //Break 할 수 있는 여부를 여기서 하드코딩으로 구현한 게 상당히 아쉽다. 시간 날때 구조를 고민해봐야 할 듯..
    //현재는 open은 항상 되고, break 여부만 달라지는데, 만약 후에 open도 못하는 경우가 생긴다면?
    const bool canDoorBreakBefore = ((HuntLevel == EBlindHuntLevel::Investigation) || (HuntLevel == EBlindHuntLevel::Chase));
    const bool canDoorBreakAfter = ((newHuntLevel == EBlindHuntLevel::Investigation) || (newHuntLevel == EBlindHuntLevel::Chase));

    
    if (canDoorBreakBefore != canDoorBreakAfter)
    {
        UE_LOG(LogPGEnemyBlind, Log, TEXT("Set OnOpenDoorColliderOverlapBegin %d"), int32(bDoorBreakOpen));
        if (canDoorBreakAfter)
        {
            SetDoorBreak(true);
        }
        else
        {
            SetDoorBreak(false);
        }
    }
    
    HuntLevel = newHuntLevel;

    UE_LOG(LogPGEnemyBlind, Log, TEXT("Set Hund Level [%d]"), int32(HuntLevel));
}

void APGBlindCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APGBlindCharacter, HuntLevel);
    DOREPLIFETIME(APGBlindCharacter, SoundState);
}

void APGBlindCharacter::BeginPlay()
{
    Super::BeginPlay();

    OnRep_SoundState();
}

void APGBlindCharacter::OnPlayerOverlapped(AActor* OverlapPlayer)
{
    if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsAttacking.IsBiting"))))
    {
        return;
    }

    Super::OnPlayerOverlapped(OverlapPlayer);
}

void APGBlindCharacter::SetSoundState(EBlindSoundState NewState)
{
    if (SoundState == NewState)
    {
        return;
    }

    // Attacking 중에는 외부에서 변경 불가(Breathing은 ResetHuntLevel에서 흐름대로)
    if (SoundState == EBlindSoundState::Attacking && NewState != EBlindSoundState::Breathing)
    {
        return;
    }

    SoundState = NewState;
    OnRep_SoundState();
}

void APGBlindCharacter::OnRep_SoundState()
{
    // 예약된 Breath, Chase 사운드 정리 (현재 재생 중인 사운드는 재생)
    GetWorldTimerManager().ClearTimer(SoundLoopTimerHandle);
    // 예약된 Growl, 진행중인 Growl 정리
    if (SoundState != EBlindSoundState::Growling)
    {
        StopGrowl();
    }

    switch (SoundState)
    {
        case EBlindSoundState::Breathing:
        {
            PlayBreathOnce();
            break;
        }

        case EBlindSoundState::Chasing:
        {
            PlayChaseOnce();
            break;
        }

        case EBlindSoundState::Growling:
        {
            GetWorldTimerManager().SetTimer(GrowlDelayTimerHandle, this, &APGBlindCharacter::PlayGrowlOnce, 1.5f, false);
            break;
        }

        case EBlindSoundState::Attacking:
        {
            if (BreathAudioComponent && BreathAudioComponent->IsPlaying())
            {
                BreathAudioComponent->FadeOut(0.3f, 0.0f);
            }
            if (ChaseAudioComponent && ChaseAudioComponent->IsPlaying())
            {
                ChaseAudioComponent->FadeOut(0.3f, 0.0f);
            }
            break;
        }

        case EBlindSoundState::Silent:
        {
            // 타이머만 정리
            break;
        }
    }
}

void APGBlindCharacter::PlayBreathOnce()
{
    if (BreathAudioComponent)
    {
        BreathAudioComponent->Play();
    }
    ScheduleNextSound();
}

void APGBlindCharacter::PlayChaseOnce()
{
    if (ChaseAudioComponent)
    {
        ChaseAudioComponent->Play();
    }
    ScheduleNextSound();
}

void APGBlindCharacter::PlayGrowlOnce()
{
    if (GrowlAudioComponent)
    {
        GrowlAudioComponent->Play();
    }
}

void APGBlindCharacter::StopGrowl()
{
    GetWorldTimerManager().ClearTimer(GrowlDelayTimerHandle);
    if (GrowlAudioComponent && GrowlAudioComponent->IsPlaying())
    {
        GrowlAudioComponent->FadeOut(0.3f, 0.0f);
    }
}

void APGBlindCharacter::ScheduleNextSound()
{
    FTimerDelegate Delegate;
    float Delay;

    switch (SoundState)
    {
        case EBlindSoundState::Breathing:
        {
            Delay = FMath::RandRange(BreathInterval.X, BreathInterval.Y);
            Delegate.BindUObject(this, &APGBlindCharacter::PlayBreathOnce);
            break;
        }

        case EBlindSoundState::Chasing:
        {
            Delay = FMath::RandRange(ChaseInterval.X, ChaseInterval.Y);
            Delegate.BindUObject(this, &APGBlindCharacter::PlayChaseOnce);
            break;
        }

        default:
        {
            return;
        }
    }

    GetWorldTimerManager().SetTimer(SoundLoopTimerHandle, Delegate, Delay, false);
}

void APGBlindCharacter::TriggerFootstepShake()
{
    if (!CameraShakeSource)
    {
        return;
    }

    switch (HuntLevel)
    {
        case EBlindHuntLevel::Exploration:
        {
            if (WalkShakeClass)
            {
                CameraShakeSource->StartCameraShake(WalkShakeClass);
            }
            break;
        }

        case EBlindHuntLevel::Investigation:
        case EBlindHuntLevel::Chase:
        {
            if (RunShakeClass)
            {
                CameraShakeSource->StartCameraShake(RunShakeClass);
            }
            break;
        }
    }
}

void APGBlindCharacter::SpawnFootprint(const FVector& Location, bool bIsLeftFoot)
{
    UMaterialInterface* FootprintDecalMaterial = bIsLeftFoot ? LeftFootprintDecalMaterial : RightFootprintDecalMaterial;
    if (!FootprintDecalMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("BlindFootprint: No DecalMaterial assigned"));
        return;
    }

    FRotator DecalRotation = GetActorRotation();
    DecalRotation.Pitch = -90.0f;
    DecalRotation.Roll = bIsLeftFoot ? 70.0f : 110.0f;

    UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
        GetWorld(),
        FootprintDecalMaterial,
        FootprintDecalSize,
        Location,
        DecalRotation,
        FootprintLifetime
    );

    if (Decal)
    {
        Decal->SetFadeScreenSize(0.0f);
        Decal->SetFadeOut(FootprintLifetime - FootprintFadeDuration, FootprintFadeDuration);
        UE_LOG(LogTemp, Log, TEXT("BlindFootprint: Spawned at %s"), *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BlindFootprint: SpawnDecalAtLocation returned null"));
    }
}
