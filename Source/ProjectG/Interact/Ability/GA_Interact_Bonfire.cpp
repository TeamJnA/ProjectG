// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_Bonfire.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBonfire.h"

UGA_Interact_Bonfire::UGA_Interact_Bonfire()
{
    //Block ability when using hands.
    HandActionLockTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
    ActivationBlockedTags.AddTag(HandActionLockTag);

    MatchTag = FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Match"));

    //Add Event trigger
    FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TriggerTag;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

    AbilityTriggers.Add(TriggerData);

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Interact_Bonfire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!PlayerCharacter)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!ASC->HasMatchingGameplayTag(MatchTag))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* TargetActor = PlayerCharacter->GetInteractionTargetActor();
    APGInteractableGimmickBonfire* Bonfire = Cast<APGInteractableGimmickBonfire>(TargetActor);
    if (Bonfire)
    {
        if (ASC->HasMatchingGameplayTag(HandActionLockTag))
        {
            UE_LOG(LogTemp, Log, TEXT("Cannot do %s during hand action."), *GetName());       
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
        PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);
        PlayerCharacter->RemoveItemFromInventory();

        Bonfire->StartBonfire();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

}

void UGA_Interact_Bonfire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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
