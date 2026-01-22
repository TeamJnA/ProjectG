// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Enemy/Ghost/AI/Controllers/PGGhostAIController.h"
#include "Perception/AISense_Touch.h"
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
}

void APGGhostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGGhostCharacter, TargetPlayerState);
    DOREPLIFETIME_CONDITION(APGGhostCharacter, bIsCurrentlyChasing, COND_Custom);
    DOREPLIFETIME_CONDITION(APGGhostCharacter, bIsCurrentlyAttacking, COND_Custom);
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

            FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsAttacking"));
            AbilitySystemComponent->RegisterGameplayTagEvent(AttackingTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APGGhostCharacter::OnAttackingTagChanged);
        }
    }
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

void APGGhostCharacter::OnChasingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    bIsCurrentlyChasing = (NewCount > 0);

    if (HasAuthority())
    {
        UpdateGhostVisibility();
    }
    ForceNetUpdate();
}

void APGGhostCharacter::OnAttackingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    bIsCurrentlyAttacking = (NewCount > 0);

    if (HasAuthority())
    {
        UpdateGhostVisibility();
    }
    ForceNetUpdate();
}

void APGGhostCharacter::OnRep_IsChasing()
{
    UpdateGhostVisibility();
}

void APGGhostCharacter::OnRep_IsAttacking()
{
    UpdateGhostVisibility();
}

void APGGhostCharacter::UpdateGhostVisibility()
{
    bool bShouldBeVisible = false;

    if (bIsCurrentlyChasing || bIsCurrentlyAttacking)
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

    const FGameplayTag ChasingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing"));
    const FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("AI.State.IsAttacking"));
    if (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(ChasingTag) || AbilitySystemComponent->HasMatchingGameplayTag(AttackingTag))
    {
        // Chasing / Attacking 상태가 아닐때 Overlap 된 경우 Jumpscare 혹은 화면 효과 + 타겟 Sanity 감소
        const float CurrentTime = GetWorld()->GetTimeSeconds();
        if (LastJumpscareTime > 0.0f && (CurrentTime - LastJumpscareTime) < JumpscareCooldown)
        {
            UE_LOG(LogTemp, Error, TEXT("[Ghost] Cool Time : %.2f"), (CurrentTime - LastJumpscareTime));
            return;
        }

        LastJumpscareTime = CurrentTime;

        if (APGPlayerController* TouchedPlayerPC = Cast<APGPlayerController>(TouchedPlayer->GetController()))
        {
            TouchedPlayerPC->Client_DisplayJumpscare(JumpscareTexture);
        }

        UAbilitySystemComponent* TargetASC = TouchedPlayer->GetAbilitySystemComponent();
        if (TargetASC)
        {
            FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
            ContextHandle.AddInstigator(this, this);

            FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(SanityDecreaseEffectClass, 1.0f, ContextHandle);
            if (SpecHandle.IsValid())
            {
                TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }

        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Ghost::OnTouchColliderOverlapBegin: Ghost touched target Starting Attack."));

    if (IAttackableTarget* AttackableInterface = Cast<IAttackableTarget>(OtherActor))
    {
        if (AttackableInterface->IsValidAttackableTarget() && OtherActor != CachedAttackedTarget)
        {
            CachedAttackedTarget = OtherActor;
            UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, OtherActor->GetActorLocation());
            AttackableInterface->OnAttacked(GetCapsuleTopWorldLocation(), 160.0f);
        }

        AbilitySystemComponent->TryActivateAbilityByClass(UGA_GhostAttack::StaticClass(), true);
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
        UE_LOG(LogTemp, Log, TEXT("Ghost::TryBindLightEffectEvents: I am NOT the target. Disabling light effect. (Local: %s, Target: %s)"),
            *GetNameSafe(LocalPlayerStateCache.Get()), *GetNameSafe(TargetPlayerState));
    }
}
