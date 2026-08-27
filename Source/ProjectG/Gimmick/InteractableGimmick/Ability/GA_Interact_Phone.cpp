// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/Ability/GA_Interact_Phone.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickPhone.h"


UGA_Interact_Phone::UGA_Interact_Phone()
{
    //Block ability when using hands.
    HandActionLockTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
    ActivationBlockedTags.AddTag(HandActionLockTag);

    //Add Event trigger
    FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TriggerTag;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

    AbilityTriggers.Add(TriggerData);

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Interact_Phone::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!PlayerCharacter)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* TargetActor = PlayerCharacter->GetInteractionTargetActor();
    if (APGInteractableGimmickPhone* Phone = Cast<APGInteractableGimmickPhone>(TargetActor))
    {
        Phone->GimmickInteract(PlayerCharacter);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_Phone::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        return;
    }
    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    ASC->ClearAbility(HandleToRemove);
}
