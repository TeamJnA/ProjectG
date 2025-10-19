// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_Revive.h"
#include "Game/PGGameMode.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

UGA_Interact_Revive::UGA_Interact_Revive()
{
	FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TriggerTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Interact_Revive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogTemp, Log, TEXT("[Revive] GA_Interact_RevivePlayer Activated."));

	APGPlayerCharacter* InteractingCharacter = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!InteractingCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("[Revive] InteractingCharacter (Reviver) is NULL."));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[Revive] Reviver: %s"), *InteractingCharacter->GetName());

	APGPlayerCharacter* DeadCharacter = Cast<APGPlayerCharacter>(const_cast<AActor*>(TriggerEventData->Target.Get()));
	if (!DeadCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("[Revive] DeadCharacter (Target) is NULL."));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[Revive] Target Ragdoll: %s"), *DeadCharacter->GetName());

	APGPlayerState* DeadPlayerState = Cast<APGPlayerState>(DeadCharacter->GetDeadPlayerState());
	if (!DeadPlayerState || !DeadPlayerState->IsDead())
	{
		UE_LOG(LogTemp, Error, TEXT("[Revive] Target's PlayerState is NULL or target is not dead."));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[Revive] Target's PlayerState is valid and marked as dead."));

	AController* DeadPlayerController = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		UE_LOG(LogTemp, Log, TEXT("[Revive] Checking Controller: %s with PlayerState: %s"), *PC->GetName(), PC->PlayerState ? *PC->PlayerState->GetPlayerName() : TEXT("NULL"));

		if (PC && PC->PlayerState == DeadPlayerState)
		{
			DeadPlayerController = PC;
			UE_LOG(LogTemp, Log, TEXT("[Revive] Found matching controller for dead player: %s"), *DeadPlayerController->GetName());

			break;
		}
	}

	if (!DeadPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("[Revive] FAILED to find the Controller for the dead player."));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APGGameMode* GM = GetWorld()->GetAuthGameMode<APGGameMode>();
	if (GM)
	{
		UE_LOG(LogTemp, Log, TEXT("[Revive] GameMode found. Calling RespawnPlayer..."));

		GM->RespawnPlayer(DeadPlayerController, DeadCharacter->GetActorTransform());
		InteractingCharacter->RemoveItemFromInventory();
		UE_LOG(LogTemp, Log, TEXT("[Revive] Destroying old ragdoll body."));

		DeadCharacter->Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Revive] GameMode is NULL."));

	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_Revive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsInstantiated())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->ClearAbility(Handle);
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
