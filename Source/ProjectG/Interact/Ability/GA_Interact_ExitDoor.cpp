// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_ExitDoor.h"
#include "AbilitySystemComponent.h"
#include "Interface/ItemInteractInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Character/PGPlayerCharacter.h"

#include "Level/Misc/PGExitDoor.h"

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

/*
* 탈출구 상호작용 처리
* ExitDoor가 이미 열린 상태(첫 탈출자가 아닌 경우) -> 플레이어 게임 종료 처리
* ExitDoor가 닫혀있고 잠긴 상태
*   플레이어가 ExitKey를 들고 있는 경우 -> ExitKey 소모 후 ExitDoor의 LockStack--
* ExitDoor가 닫혀있고 잠김이 풀린 상태(첫 탈출자) -> ExitDoor을 열고 플레이어 게임 종료 처리
*/
void UGA_Interact_ExitDoor::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // --- check valid ---
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
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
    // --- check valid ---

    if (ExitDoor->IsLocked())
    {
        UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Exit Door is locked."));

        if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ExitKey"))))
        {
            if (ASC->HasMatchingGameplayTag(HandActionLockTag))
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot do %s during hand action."), *GetName());
                return;
            }

            PGCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);
            PGCharacter->RemoveItemFromInventory();
            ExitDoor->SubtractLockStack();
        }
    }
    else if (!ExitDoor->IsOpened())
    {
        UE_LOG(LogTemp, Log, TEXT("UGA_Interact_ExitDoor::ActivateAbility: Exit door is unlocked. Open exit door"));
        ExitDoor->ToggleDoor();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_ExitDoor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
