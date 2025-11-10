// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Interface/AttackableTarget.h"
#include "Perception/AISense_Touch.h"
#include "AbilitySystemComponent.h"

APGGhostCharacter::APGGhostCharacter()
{
    ExplorationRadius = 3000.0f;
    ExplorationWaitTime = 5.0f;
    bIsCurrentlyChasing = false;

    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(true);
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
}

void APGGhostCharacter::SetTargetPlayerState(APlayerState* InPlayerState)
{
    if (HasAuthority())
    {
        if (InPlayerState)
        {
            UE_LOG(LogTemp, Log, TEXT("Ghost::SetTargetPlayerState"));
            TargetPlayerState = InPlayerState;
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
