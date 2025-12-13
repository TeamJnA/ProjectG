// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/Ability/GA_Interact_Lever.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickLever.h"
#include "Character/PGPlayerCharacter.h"
#include "AbilitySystemComponent.h"

UGA_Interact_Lever::UGA_Interact_Lever()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked")));
}

void UGA_Interact_Lever::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* TargetActorConst = TriggerEventData->Target.Get();
	AActor* TargetActor = const_cast<AActor*>(TargetActorConst);
	APGInteractableGimmickLever* Lever = Cast<APGInteractableGimmickLever>(TargetActor);

	if (Lever)
	{
		if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			Player->PlayHandActionAnimMontage(EHandActionMontageType::Pick);
		}

		Lever->ActivateLever();
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_Lever::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->ClearAbility(Handle);
	}
}
