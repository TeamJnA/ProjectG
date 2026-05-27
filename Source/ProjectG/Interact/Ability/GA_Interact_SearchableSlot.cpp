// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Interact_SearchableSlot.h"
#include "Character/PGPlayerCharacter.h"
#include "Level/Searchable/PGSearchableSlotBase.h"
#include "AbilitySystemComponent.h"


UGA_Interact_SearchableSlot::UGA_Interact_SearchableSlot()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked")));
}

void UGA_Interact_SearchableSlot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* TargetActorConst = TriggerEventData->Target.Get();
	AActor* TargetActor = const_cast<AActor*>(TargetActorConst);
	APGSearchableSlotBase* SSBase = Cast<APGSearchableSlotBase>(TargetActor);
	if (SSBase)
	{
		if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			Player->Server_PlayHandActionAnimMontage(EHandActionMontageType::Pick);
		}

		SSBase->InteractSlot();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Interact_SearchableSlot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->ClearAbility(Handle);
	}
}
