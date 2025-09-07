// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_ExitDoor.h"
#include "AbilitySystemComponent.h"
#include "Interface/ItemInteractInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "Game/PGGameState.h"
#include "Character/PGPlayerCharacter.h"

#include "Level/PGExitDoor.h"

UGA_Interact_ExitDoor::UGA_Interact_ExitDoor()
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

void UGA_Interact_ExitDoor::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	PG_CHECK_VALID_INTERACT(ASC);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	PG_CHECK_VALID_INTERACT(AvatarActor);

    if (IsValid(TriggerEventData->Target))
    {
        UE_LOG(LogTemp, Log, TEXT("Activate interact %s ability to %s"), *TriggerEventData->Target.GetFName().ToString(), *GetOwningActorFromActorInfo()->GetName());
    }

    UE_LOG(LogTemp, Warning, TEXT("GA_Interact_ExitDoor::ActivateAbility - Called. IsLocalController: %d, HasAuthority: %d"),
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

    APGExitDoor* ExitDoor = Cast<APGExitDoor>(TargetActor);
    PG_CHECK_VALID_INTERACT(ExitDoor);
    
    /*
    * if exit door is already opened (not a first escape player)
    * process player finish
    */
    if (ExitDoor->IsOpened())
    {
        HandlePlayerFinished(PGCharacter);
    }
    /*
    * if exit door is locked
    * player should find 3 exit key and use each one to unlock the exit door
    */
    else if (ExitDoor->IsLocked())
    {
        /*
        * if exit door is locked
        */
        UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Exit Door is locked."));

        // check player has exit key in hand
        if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ExitKey"))))
        {
            // block entry when hand action is in progress
            if (ASC->HasMatchingGameplayTag(HandActionLockTag))
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot do %s during hand action."), *GetName());
                return;
            }

            // play hand action(unlock motion)
            PGCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);
            // remove exit key item after use
            PGCharacter->RemoveItemFromInventory();
            ExitDoor->SubtractLockStack();
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Player doesnt have exit key."));
        }
    }
    /*
    * if door is unlocked but not opened (first escape player)
    * open door and process player finish
    */
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Exit door is unlocked. Open exit door"));
        ExitDoor->ToggleDoor();
        HandlePlayerFinished(PGCharacter);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_ExitDoor::HandlePlayerFinished(APGPlayerCharacter* PGCharacter)
{
    if (!PGCharacter)
    {
        return;
    }

    APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
    APlayerState* PS = PGCharacter->GetPlayerState();
    if (GS && PS)
    {
        GS->MarkPlayerAsFinished(PS);

        if (GS->IsGameFinished())
        {
            GS->SetCurrentGameState(EGameState::EndGame);
            GS->NotifyGameFinished();
        }
        else
        {
            PGCharacter->Client_InitScoreBoardWidget();
        }
    }
}

void UGA_Interact_ExitDoor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
