// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_LobbyDoor.h"

#include "Kismet/GameplayStatics.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

#include "Game/PGLobbyGameMode.h"
#include "Player/PGPlayerState.h"
#include "Character/PGPlayerCharacter.h"

#include "Level/PGLobbyDoor.h"


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

void UGA_Interact_LobbyDoor::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    ensureMsgf(AvatarActor, TEXT("GA_Interact_LobbyDoor::ActivateAbility: Avatar actor is not valid"));
    APawn* AvatarPawn = Cast<APawn>(AvatarActor);
    ensureMsgf(AvatarPawn, TEXT("GA_Interact_LobbyDoor::ActivateAbility: Avatar pawn is not valid"));
    APGPlayerState* PS = AvatarPawn->GetPlayerState<APGPlayerState>();
    ensureMsgf(PS, TEXT("GA_Interact_LobbyDoor::ActivateAbility: PS is not valid"));

    if (PS->IsHost())
    {
        UWorld* World = GetWorld();
        ensure(World);
        AGameModeBase* GM = UGameplayStatics::GetGameMode(World);
        ensure(GM);
        APGLobbyGameMode* LobbyGM = Cast<APGLobbyGameMode>(GM);
        ensure(LobbyGM);

        LobbyGM->StartGame();        
    }

    //if (ActorInfo->IsNetAuthority())
    //{
    //    UWorld* World = GetWorld();
    //    ensure(World);
    //    AGameModeBase* GM = UGameplayStatics::GetGameMode(World);
    //    ensure(GM);
    //    APGLobbyGameMode* LobbyGM = Cast<APGLobbyGameMode>(GM);
    //    ensure(LobbyGM);

    //    LobbyGM->StartGame();
    //}

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
