// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_LobbyDoor.h"

#include "Kismet/GameplayStatics.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "Game/PGLobbyGameMode.h"
#include "Player/PGPlayerState.h"

#include "Level/Misc/PGLobbyDoor.h"

UGA_Interact_LobbyDoor::UGA_Interact_LobbyDoor()
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
* 상호작용을 시도한 플레이어가 호스트인 경우 StartGame 
*/
void UGA_Interact_LobbyDoor::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    const APGPlayerState* PS = Cast<APGPlayerState>(ActorInfo->OwnerActor.Get());
    // PlayerState .h
    // bool IsHost() const { return bIsHost; }
    if (PS && PS->IsHost())
    {
        if (APGLobbyGameMode* LobbyGM = Cast<APGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
        {
            LobbyGM->StartGame();
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Interact_LobbyDoor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ensure(ASC);
    ASC->ClearAbility(HandleToRemove);
}
