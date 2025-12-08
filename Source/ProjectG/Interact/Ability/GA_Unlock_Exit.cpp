// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Unlock_Exit.h"
#include "GA_Unlock_Exit.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Character/PGPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "PGLogChannels.h"

UGA_Unlock_Exit::UGA_Unlock_Exit()
{
	HandActionLockTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
	ActivationBlockedTags.AddTag(HandActionLockTag);

	FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TriggerTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Unlock_Exit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // --- check valid ---
    const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    PG_CHECK_VALID_INTERACT(ASC);

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    PG_CHECK_VALID_INTERACT(AvatarActor);

    if (IsValid(TriggerEventData->Target))
    {
        UE_LOG(LogPGAbility, Log, TEXT("Activate interact %s ability to %s"), *TriggerEventData->Target.GetFName().ToString(), *GetOwningActorFromActorInfo()->GetName());
    }

    APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(AvatarActor);
    PG_CHECK_VALID_INTERACT(PGCharacter);

    AActor* TargetActor = PGCharacter->GetInteractionTargetActor();
    PG_CHECK_VALID_INTERACT(TargetActor);

    if (TargetActor != TriggerEventData->Target.Get()) {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        UE_LOG(LogTemp, Log, TEXT("TargetActor does not match the payload target"));
        return;
    }

    APGExitPointBase* PGExitPoint = Cast<APGExitPointBase>(TargetActor);
    PG_CHECK_VALID_INTERACT(PGExitPoint);
    // --- check valid ---

    if (ASC->HasMatchingGameplayTag(HandActionLockTag))
    {
        UE_LOG(LogTemp, Log, TEXT("Cannot do %s during hand action."), *GetName());
        return;
    }
    PGCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);

    if (PGExitPoint->Unlock())
    {
        PGCharacter->RemoveItemFromInventory();
    }
}

void UGA_Unlock_Exit::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
