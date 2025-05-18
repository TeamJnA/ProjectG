// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Ability/Chase/GA_BlindChase.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"

UGA_BlindChase::UGA_BlindChase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	SetAssetTags(TagContainer);
	
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));


	static ConstructorHelpers::FObjectFinder<UClass> PickGERef(TEXT("/Game/ProjectG/Enemy/Blind/Ability/Effect/GE_BlindChase.GE_BlindChase_C"));
	if (PickGERef.Object)
	{
		ChaseEffectClass = PickGERef.Object;
	}
}

void UGA_BlindChase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ChaseEffectClass)
	{
		UE_LOG(LogTemp, Log, TEXT("DoChase"));
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ChaseEffectClass);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}

	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());
	OwnerPawn->SetHuntLevel(2);


	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));
}

void UGA_BlindChase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));
}
