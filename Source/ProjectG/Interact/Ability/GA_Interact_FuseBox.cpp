// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_FuseBox.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Gimmick/InteractableGimmick/PGFuseBox.h"

UGA_Interact_FuseBox::UGA_Interact_FuseBox()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);

    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked")));
}

void UGA_Interact_FuseBox::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!HasAuthority(&ActivationInfo) || !TriggerEventData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APGFuseBox* FuseBox = Cast<APGFuseBox>(const_cast<AActor*>(TriggerEventData->Target.Get()));
    if (!FuseBox)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FuseBox->OpenBox();

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_FuseBox::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
    {
        ASC->ClearAbility(Handle);
    }
}
