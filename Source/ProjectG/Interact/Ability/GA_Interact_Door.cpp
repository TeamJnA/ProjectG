// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_Door.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Interface/ItemInteractInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "Level/Misc/PGDoor1.h"

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

    // 어빌리티가 클라이언트에서 호출되었을 때, NetExecutionPolicy::ServerOnly에 의해 실제로 서버로 넘어가는지 확인하는 로그
    UE_LOG(LogTemp, Warning, TEXT("GA_Interact_Door::ActivateAbility - Called. IsLocalController: %d, HasAuthority: %d"),
        ActorInfo->IsLocallyControlled(), HasAuthority(&CurrentActivationInfo));

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

            /*
            * If door is locked and player has key on hand
            * Start hand action tag
            * Remove key item
            */
            // Play open door animation montage
            PGCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);

            if (HasAuthority(&CurrentActivationInfo))
            {
                PGCharacter->RemoveItemFromInventory();
            }

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
        // Play open door animation montage

        PGCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);

        UE_LOG(LogTemp, Warning, TEXT("GA_Interact_Door::ActivateAbility - Calling Door->ToggleDoor() for %s."), *GetNameSafe(Door));
        Door->ToggleDoor(AvatarActor);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }    
}

void UGA_Interact_Door::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
