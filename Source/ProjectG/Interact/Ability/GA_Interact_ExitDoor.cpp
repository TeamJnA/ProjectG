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
    * if exit door is already opened
    * just end ability
    */
    if (ExitDoor->IsOpened())
    {
        UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Exit door already opened"));

        APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
        if (GS)
        {
            GS->IncreaseFinishedPlayersCount();

            if (GS->IsGameFinished())
            {
                UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Game Finished"));

                GS->SetCurrentGameState(EGameState::EndGame);
                GS->NotifyGameFinished();
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UGA_Interact_ExitDoor::ActivateAbility: No GS"));
        }

        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    if (ExitDoor->IsLocked())
    {
        /*
        * if exit door is locked
        * player should find 3 exit key and use each one to unlock the exit door
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
            if (HasAuthority(&CurrentActivationInfo))
            {
                PGCharacter->RemoveItemFromInventory();
            }

            ExitDoor->SubtractLockStack();

            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Player doesnt have exit key."));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Exit door is unlocked. Open exit door"));
        ExitDoor->ToggleDoor();
        /*
        * if exit door is unlocked
        * end game
        * show score board, enter spectator mode
        */
        // PGCharacter->InitScoreboard
        // InitScoreboard->SpectateButton bind to enter spectate mode

        PGCharacter->Client_InitScoreBoardWidget();
        
        APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
        if (GS)
        {
            GS->IncreaseFinishedPlayersCount();

            if (GS->IsGameFinished())
            {
                UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Game Finished"));

                GS->SetCurrentGameState(EGameState::EndGame);
                GS->NotifyGameFinished();
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UGA_Interact_ExitDoor::ActivateAbility: No GS"));
        }

        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UGA_Interact_ExitDoor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
