// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_ExitDoor.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Interface/ItemInteractInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

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

    // 어빌리티가 클라이언트에서 호출되었을 때, NetExecutionPolicy::ServerOnly에 의해 실제로 서버로 넘어가는지 확인하는 로그
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

    if (ExitDoor->IsLocked())
    {
        // Check ExitDoor Key Tag
        ExitDoor->SubtractLockStack();
    }
    else
    {
        
    }
}

void UGA_Interact_ExitDoor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}
