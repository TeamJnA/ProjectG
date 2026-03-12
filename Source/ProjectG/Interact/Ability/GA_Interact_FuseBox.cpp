// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_FuseBox.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Gimmick/InteractableGimmick/PGFuseBox.h"

UGA_Interact_FuseBox::UGA_Interact_FuseBox()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = EventTag;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);
}

void UGA_Interact_FuseBox::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
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
