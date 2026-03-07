// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Interact_GimmickBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "AbilitySystemComponent.h"


UGA_Interact_GimmickBase::UGA_Interact_GimmickBase()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked")));
}

void UGA_Interact_GimmickBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* TargetActorConst = TriggerEventData->Target.Get();
	AActor* TargetActor = const_cast<AActor*>(TargetActorConst);
	APGInteractableGimmickBase* IGBase = Cast<APGInteractableGimmickBase>(TargetActor);
	if (IGBase)
	{
		if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			Player->PlayHandActionAnimMontage(EHandActionMontageType::Pick);
		}

		IGBase->GimmickInteract();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_GimmickBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->ClearAbility(Handle);
	}
}
