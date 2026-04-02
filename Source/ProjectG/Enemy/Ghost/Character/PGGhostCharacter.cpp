// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Ghost/Ability/Attack/GA_GhostAttack.h"

#include "Interface/AttackableTarget.h"
#include "Interface/LightEffectInterface.h"

#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

APGGhostCharacter::APGGhostCharacter()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(true);

    LightExtinguishSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LightExtinguishSphere"));
    LightExtinguishSphere->SetupAttachment(RootComponent);
    LightExtinguishSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 280.0f));
    LightExtinguishSphere->SetSphereRadius(700.0f);
    LightExtinguishSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LightExtinguishSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    LightExtinguishSphere->SetGenerateOverlapEvents(false);

    HeadlightFlickerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HeadlightFlickerSphere"));
    HeadlightFlickerSphere->SetupAttachment(RootComponent);
    HeadlightFlickerSphere->SetSphereRadius(1400.0f);
    HeadlightFlickerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeadlightFlickerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    HeadlightFlickerSphere->SetGenerateOverlapEvents(false);
}

void APGGhostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGGhostCharacter, TargetPlayerState);
    DOREPLIFETIME(APGGhostCharacter, CurrentGhostState);
}

void APGGhostCharacter::BeginPlay()
{
    Super::BeginPlay();

    UpdateGhostVisibility();

    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (LocalPC)
    {
        LocalPlayerStateCache = LocalPC->PlayerState;
        UE_LOG(LogTemp, Warning, TEXT("APGGhostCharacter::BeginPlay [%s]: LocalPlayerStateCache: %s"),
            HasAuthority() ? TEXT("SERVER (HOST)") : TEXT("CLIENT"),
            *GetNameSafe(LocalPlayerStateCache.Get()));
    }

    TryBindLightEffectEvents();
}

void APGGhostCharacter::SetTargetPlayerState(APlayerState* InPlayerState)
{
    if (HasAuthority())
    {
        if (InPlayerState)
        {
            UE_LOG(LogTemp, Log, TEXT("Ghost::SetTargetPlayerState"));
            TargetPlayerState = InPlayerState;

            APGGhostAIController* AIC = Cast<APGGhostAIController>(GetController());
            if (AIC)
            {
                AIC->SetupTarget(TargetPlayerState);
            }
            TryBindLightEffectEvents();
        }
    }
}

bool APGGhostCharacter::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    if (!TargetPlayerState)
    {
        return false;
    }

    const APlayerController* PC = Cast<APlayerController>(RealViewer);
    if (PC && PC->PlayerState == TargetPlayerState)
    {
        return true;
    }
    return false;
}

bool APGGhostCharacter::IsPhotographable() const
{
    APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!LocalPC || !LocalPC->PlayerState)
    {
        return false;
    }

    if (LocalPC->PlayerState != TargetPlayerState)
    {
        return false;
    }

    return true;
}

FPhotoSubjectInfo APGGhostCharacter::GetPhotoSubjectInfo() const
{
    int32 ID = 0;
    int32 Score = 0;

    switch (CurrentGhostState)
    {
        case E_PGGhostState::Waiting:
        case E_PGGhostState::Exploring:
        {
            ID = PhotoID::Ghost_Exploring;
            Score = 10;
            break;
        }

        case E_PGGhostState::Tracking:
        case E_PGGhostState::Chasing:
        case E_PGGhostState::Attacking:
        {
            ID = PhotoID::Ghost_Chasing;
            Score = 70;
            break;
        }
    }

    return FPhotoSubjectInfo(ID, Score);
}

FVector APGGhostCharacter::GetPhotoTargetLocation() const
{
    return GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
}

void APGGhostCharacter::SetCameraModeVisible(bool bVisible)
{
    bCameraModeVisible = bVisible;
    UpdateGhostVisibility();
}

void APGGhostCharacter::SetGhostState(E_PGGhostState NewState)
{
    if (HasAuthority())
    {
        CurrentGhostState = NewState;
        UpdateGhostVisibility();
        ForceNetUpdate();
    }
}

void APGGhostCharacter::OnRep_GhostState()
{
    UpdateGhostVisibility();
}

void APGGhostCharacter::UpdateGhostVisibility()
{
    bool bShouldBeVisible = false;

    if (CurrentGhostState == E_PGGhostState::Chasing || CurrentGhostState == E_PGGhostState::Attacking)
    {
        bShouldBeVisible = true;
    }

    if (bCameraModeVisible)
    {
        bShouldBeVisible = true;
    }

    if (HasAuthority())
    {
        APlayerController* HostPC = GetWorld()->GetFirstPlayerController();
        if (HostPC && HostPC->PlayerState != TargetPlayerState)
        {
            bShouldBeVisible = false;
        }
    }

    GetMesh()->SetVisibility(bShouldBeVisible);
}

void APGGhostCharacter::OnTouchColliderOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    APGPlayerCharacter* TouchedPlayer = Cast<APGPlayerCharacter>(OtherActor);
    if (!TouchedPlayer || TouchedPlayer->GetPlayerState() != TargetPlayerState)
    {
        return;
    }

    // Chasing 상태에서만 공격
    if (CurrentGhostState == E_PGGhostState::Chasing)
    {
        if (IAttackableTarget* AttackableInterface = Cast<IAttackableTarget>(OtherActor))
        {
            if (AttackableInterface->IsValidAttackableTarget() && OtherActor != CachedAttackedTarget)
            {
                CachedAttackedTarget = OtherActor;
                AttackableInterface->OnAttacked(GetCapsuleTopWorldLocation(), 160.0f);
            }

            AbilitySystemComponent->TryActivateAbilityByClass(UGA_GhostAttack::StaticClass(), true);
        }
        return;
    }

    if (CurrentGhostState == E_PGGhostState::Attacking || CurrentGhostState == E_PGGhostState::Tracking)
    {
        return;
    }

    // Exploring/Waiting 상태에서 터치 → 글리치 + Sanity 감소
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (LastJumpscareTime > 0.0f && (CurrentTime - LastJumpscareTime) < JumpscareCooldown)
    {
        UE_LOG(LogTemp, Error, TEXT("[Ghost] Cool Time : %.2f"), (CurrentTime - LastJumpscareTime));
        return;
    }

    LastJumpscareTime = CurrentTime;
    TouchedPlayer->Client_TriggerGhostGlitch();

    UAbilitySystemComponent* TargetASC = TouchedPlayer->GetAbilitySystemComponent();
    if (TargetASC && SanityDecreaseEffectClass)
    {
        FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
        ContextHandle.AddInstigator(this, this);

        FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(SanityDecreaseEffectClass, 1.0f, ContextHandle);
        if (SpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }
}

void APGGhostCharacter::OnLightExtinguishOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || !OtherComp)
    {
        return;
    }

    if (ILightEffectInterface* FadableTarget = Cast<ILightEffectInterface>(OtherComp))
    {
        FadableTarget->FadeOut();
    }
}

void APGGhostCharacter::OnLightExtinguishOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this || !OtherComp)
    {
        return;
    }

    if (ILightEffectInterface* FadableTarget = Cast<ILightEffectInterface>(OtherComp))
    {
        FadableTarget->FadeIn();
    }
}

void APGGhostCharacter::OnFlickerSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || !OtherComp)
    {
        return;
    }

    if (APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(OtherActor))
    {
        if (PGCharacter->GetPlayerState() == TargetPlayerState)
        {
            PGCharacter->EnterGhostZone(this);
        }
    }
}

void APGGhostCharacter::OnFlickerSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this || !OtherComp)
    {
        return;
    }

    if (APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(OtherActor))
    {
        if (PGCharacter->GetPlayerState() == TargetPlayerState)
        {
            PGCharacter->ExitGhostZone();
        }
    }
}

void APGGhostCharacter::TryBindLightEffectEvents()
{
    UE_LOG(LogTemp, Log, TEXT("Ghost::TryBindLightEffectEvents: LocalPlayerStateCache: %d, TargetPlayerState: %d"),
        LocalPlayerStateCache.IsValid(), IsValid(TargetPlayerState));

    if (!LocalPlayerStateCache.IsValid() || !IsValid(TargetPlayerState))
    {
        return;
    }

    if (LocalPlayerStateCache.Get() == TargetPlayerState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ghost::TryBindLightEffectEvents: I AM THE TARGET. Binding overlap events. (Local: %s, Target: %s)"),
            *GetNameSafe(LocalPlayerStateCache.Get()), *GetNameSafe(TargetPlayerState));

        // Light/Emissive용
        LightExtinguishSphere->OnComponentBeginOverlap.AddDynamic(this, &APGGhostCharacter::OnLightExtinguishOverlapBegin);
        LightExtinguishSphere->OnComponentEndOverlap.AddDynamic(this, &APGGhostCharacter::OnLightExtinguishOverlapEnd);
        LightExtinguishSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
        LightExtinguishSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        LightExtinguishSphere->SetGenerateOverlapEvents(true);

        // 캐릭터 헤드라이트 플리커용
        HeadlightFlickerSphere->OnComponentBeginOverlap.AddDynamic(this, &APGGhostCharacter::OnFlickerSphereOverlapBegin);
        HeadlightFlickerSphere->OnComponentEndOverlap.AddDynamic(this, &APGGhostCharacter::OnFlickerSphereOverlapEnd);
        HeadlightFlickerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        HeadlightFlickerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        HeadlightFlickerSphere->SetGenerateOverlapEvents(true);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Ghost::TryBindLightEffectEvents: I am NOT the target. Disabling light effect. (Local: %s, Target: %s)"),
            *GetNameSafe(LocalPlayerStateCache.Get()), *GetNameSafe(TargetPlayerState));
    }
}
