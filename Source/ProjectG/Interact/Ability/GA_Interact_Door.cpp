// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_Door.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Interface/ItemInteractInterface.h"
#include "Level/PGDoor1.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

UGA_Interact_Door::UGA_Interact_Door()
{
    HandActionLockTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
    //Add Event trigger
    FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
    //Block ability when using hands.
    ActivationBlockedTags.AddTag(HandActionLockTag);

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TriggerTag;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

    AbilityTriggers.Add(TriggerData);

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Interact_Door::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	PG_CHECK_VALID_INTERACT(ASC);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	PG_CHECK_VALID_INTERACT(AvatarActor);

    if (IsValid(TriggerEventData->Target))
    {
        UE_LOG(LogTemp, Log, TEXT("Activate interact %s ability to %s"), *TriggerEventData->Target.GetFName().ToString(), *GetOwningActorFromActorInfo()->GetName());
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

    APGDoor1* Door = Cast<APGDoor1>(TargetActor);
    PG_CHECK_VALID_INTERACT(Door);

    /*
    * if door is locked
    *   if player has key -> toggle door, destroy key
    *   if player does not have key -> end ability
    * if door is unlocked -> toggle door
    */
    if (Door->IsLocked())
    {
        UE_LOG(LogTemp, Log, TEXT("Door is locked."));
        if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Key"))))
        {
            UE_LOG(LogTemp, Log, TEXT("Unlock door."));

            if (ASC->HasMatchingGameplayTag(HandActionLockTag))
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot do %s during hand action."), *GetName());
                return;
            }

            // Set the Pick Item montage in player character and activate GA_HandAction to play pick item anim.
            PGCharacter->SetHandActionAnimMontage(EHandActionMontageType::Pick);

            FGameplayTag HandActionTag = FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction"));

            FGameplayTagContainer HandActionTagContainer;
            HandActionTagContainer.AddTag(HandActionTag);

            PGCharacter->ActivateAbilityByTag(HandActionTagContainer);

            UAbilityTask_WaitGameplayTagRemoved* WaitTask = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
                this,
                FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction"))
            );

            WaitTask->Removed.AddDynamic(this, &UGA_Interact_Door::OnHandActionEnd);
            WaitTask->ReadyForActivation();

            Door->UnLock();
            /*
            * after hand action end -> OnHandActionEnd -> Remove item 
            */
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("No key"));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        }
    }
    else
    {
        if (ASC->HasMatchingGameplayTag(HandActionLockTag))
        {
            UE_LOG(LogTemp, Log, TEXT("Cannot do %s during hand action."), *GetName());
            return;
        }
        PGCharacter->SetHandActionAnimMontage(EHandActionMontageType::Pick);

        FGameplayTag HandActionTag = FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction"));

        FGameplayTagContainer HandActionTagContainer;
        HandActionTagContainer.AddTag(HandActionTag);

        PGCharacter->ActivateAbilityByTag(HandActionTagContainer);

        // Active hand action tag and play animation montage
        Door->ToggleDoor();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }    
}

void UGA_Interact_Door::OnHandActionEnd()
{
    // Remove item. Inventory is replicated, so remove item also only on the server.
    if (HasAuthority(&CurrentActivationInfo))
    {
        AActor* AvatarActor = GetAvatarActorFromActorInfo();
        APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(AvatarActor);
        if (!PGCharacter) {
            UE_LOG(LogTemp, Warning, TEXT("Cannot found avatar actor in RemoveItem %s"), *GetName());
            return;
        }

        PGCharacter->RemoveItemFromInventory();
    }
}


void UGA_Interact_Door::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
