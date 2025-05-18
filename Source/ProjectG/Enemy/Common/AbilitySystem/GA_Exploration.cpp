// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "AbilitySystemComponent.h"


UGA_Exploration::UGA_Exploration()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Exploration")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Exploration")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Exploration")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));

	
	



	static ConstructorHelpers::FObjectFinder<UClass> PickGERef(TEXT("/Game/ProjectG/Enemy/Common/Abilities/GE_Exploration.GE_Exploration_C"));
	if (PickGERef.Object) 
	{ ExplorationEffectClass = PickGERef.Object; }

}

void UGA_Exploration::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	

	if (ExplorationEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ExplorationEffectClass);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}

	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsExploring")));



}

void UGA_Exploration::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("EndExploration"));
	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsExploring")));
}

void UGA_Exploration::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// Ability �ڵ� ����
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		const bool bSuccess = ASC->TryActivateAbility(Spec.Handle);
	}
}

