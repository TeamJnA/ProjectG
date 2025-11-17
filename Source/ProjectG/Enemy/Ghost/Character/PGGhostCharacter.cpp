// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Perception/AISense_Touch.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"

#include "Interface/AttackableTarget.h"
#include "Interface/LightEffectInterface.h"

#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

APGGhostCharacter::APGGhostCharacter()
{
    ExplorationRadius = 3000.0f;
    ExplorationWaitTime = 5.0f;
    bIsCurrentlyChasing = false;

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
}

void APGGhostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGGhostCharacter, TargetPlayerState);
    DOREPLIFETIME_CONDITION(APGGhostCharacter, bIsCurrentlyChasing, COND_Custom);
}

void APGGhostCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        if (AbilitySystemComponent)
        {
            FGameplayTag ChasingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing"));
            AbilitySystemComponent->RegisterGameplayTagEvent(ChasingTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APGGhostCharacter::OnChasingTagChanged);
        }

        APlayerController* HostPC = GetWorld()->GetFirstPlayerController();
        if (HostPC && HostPC->PlayerState != TargetPlayerState)
        {
            GetMesh()->SetVisibility(false);
        }
        else
        {
            GetMesh()->SetVisibility(bIsCurrentlyChasing);
        }
    }
    else
    {
        GetMesh()->SetVisibility(bIsCurrentlyChasing);
    }

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
                UE_LOG(LogTemp, Log, TEXT("APGGhostCharacter::SetTargetPlayerState: set target ps to controller."));
                AIC->SetupTarget(TargetPlayerState);
            }
			UE_LOG(LogTemp, Log, TEXT("Ghost::SetTargetPlayerState: TryStartLightEffectTimer"));
            TryBindLightEffectEvents();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Ghost::SetTargetPlayerState: Failed"));
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

void APGGhostCharacter::OnChasingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    bIsCurrentlyChasing = (NewCount > 0);

    if (HasAuthority())
    {
        APlayerController* HostPC = GetWorld()->GetFirstPlayerController();
        if (HostPC && HostPC->PlayerState == TargetPlayerState)
        {
            OnRep_IsChasing();
        }
    }

    ForceNetUpdate();
}

void APGGhostCharacter::OnRep_IsChasing()
{
    GetMesh()->SetVisibility(bIsCurrentlyChasing);
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

    const FGameplayTag ChasingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing"));
    if (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(ChasingTag))
    {
        return;
    }

    APGPlayerCharacter* TouchedPlayer = Cast<APGPlayerCharacter>(OtherActor);
    if (!TouchedPlayer)
    {
        return;
    }

    APlayerState* TouchedPlayerState = TouchedPlayer->GetPlayerState();
    if (!TouchedPlayerState || TouchedPlayerState != TargetPlayerState)
    {
        return;
    }

    if (IAttackableTarget* AttackableInterface = Cast<IAttackableTarget>(OtherActor))
    {
        if (AttackableInterface->IsValidAttackableTarget() && OtherActor != CachedAttackedTarget)
        {
            CachedAttackedTarget = OtherActor;
            UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, OtherActor->GetActorLocation());
            AttackableInterface->OnAttacked(GetCapsuleTopWorldLocation());
        }
    }
}

void APGGhostCharacter::OnLightExtinguishOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || !OtherComp)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Ghost::OnLightExtinguishOverlapBegin: Overlapped with Component: %s (Owner: %s)"), *GetNameSafe(OtherComp), *GetNameSafe(OtherActor));

    if (ILightEffectInterface* FadableTarget = Cast<ILightEffectInterface>(OtherComp))
    {
        UE_LOG(LogTemp, Log, TEXT("Ghost::OnLightExtinguishOverlapBegin: Component (%s) implements ILightEffectInterface. Calling FadeOut()."), *OtherComp->GetName());
        FadableTarget->FadeOut();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Ghost::OnLightExtinguishOverlapBegin: Overlap Begin object (%s or %s) does NOT implement ILightEffectInterface."), 
            *OtherActor->GetName(), *OtherComp->GetName());
    }
}

void APGGhostCharacter::OnLightExtinguishOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this || !OtherComp)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Ghost::OnLightExtinguishOverlapEnd: Overlap END with Component: %s (Owner: %s)"), *GetNameSafe(OtherComp), *GetNameSafe(OtherActor));

    if (ILightEffectInterface* FadableTarget = Cast<ILightEffectInterface>(OtherComp))
    {
        UE_LOG(LogTemp, Warning, TEXT("Ghost::OnLightExtinguishOverlapEnd: Component (%s) implements ILightEffectInterface. Calling FadeIn()."), *OtherComp->GetName());
        FadableTarget->FadeIn();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Ghost::OnLightExtinguishOverlapEnd: Overlap End object (%s or %s) does NOT implement ILightEffectInterface."), 
            *OtherActor->GetName(), *OtherComp->GetName());
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

    UE_LOG(LogTemp, Log, TEXT("Ghost::TryBindLightEffectEvents: bind start"));

    if (LocalPlayerStateCache.Get() == TargetPlayerState)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryBindLightEffectEvents: I AM THE TARGET. Binding overlap events. (Local: %s, Target: %s)"),
            *GetNameSafe(LocalPlayerStateCache.Get()), *GetNameSafe(TargetPlayerState));

        LightExtinguishSphere->OnComponentBeginOverlap.AddDynamic(this, &APGGhostCharacter::OnLightExtinguishOverlapBegin);
        LightExtinguishSphere->OnComponentEndOverlap.AddDynamic(this, &APGGhostCharacter::OnLightExtinguishOverlapEnd);

        LightExtinguishSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
        LightExtinguishSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
        LightExtinguishSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
        LightExtinguishSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        LightExtinguishSphere->SetGenerateOverlapEvents(true);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("TryBindLightEffectEvents: I am NOT the target. Disabling light effect. (Local: %s, Target: %s)"),
            *GetNameSafe(LocalPlayerStateCache.Get()), *GetNameSafe(TargetPlayerState));
    }
}
