// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Ability/Investigate/GA_BlindInvestigate.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"

UGA_BlindInvestigate::UGA_BlindInvestigate()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Investigate")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Investigate")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Investigate")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));

	
	static ConstructorHelpers::FObjectFinder<UClass> PickGERef(TEXT("/Game/ProjectG/Enemy/Blind/Ability/Effect/GE_BlindInvestigate.GE_BlindInvestigate_C"));
	if (PickGERef.Object)
	{
		InvestigateEffectClass = PickGERef.Object;
	}
}

void UGA_BlindInvestigate::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (InvestigateEffectClass)
	{
		UE_LOG(LogTemp, Log, TEXT("DoInvestigate"));
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(InvestigateEffectClass);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}

	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());
	OwnerPawn->SetHuntLevel(1);

	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsInvestigating")));
}

void UGA_BlindInvestigate::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsInvestigating")));
}
