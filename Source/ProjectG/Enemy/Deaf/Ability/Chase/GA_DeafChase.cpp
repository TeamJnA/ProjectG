// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Deaf/Ability/Chase/GA_DeafChase.h"
#include "Enemy/Deaf/Character/PGDeafCharacter.h"
#include "Enemy/Deaf/AI/Controllers/PGDeafAIController.h"

#include "AbilitySystemComponent.h"

UGA_DeafChase::UGA_DeafChase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));

	static ConstructorHelpers::FObjectFinder<UClass> ChaseGERef(TEXT("/Game/ProjectG/Enemy/Deaf/Ability/Effect/GE_DeafChase.GE_DeafChase_C"));
	if (ChaseGERef.Object)
	{
		ChaseEffectClass = ChaseGERef.Object;
	}
}

void UGA_DeafChase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APGDeafCharacter* OwnerPawn = Cast<APGDeafCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find OwnerPawn in UGA_DeafChase::ActivateAbility"));
		return;
	}

	// Apply chase state gameplay effect.
	if (ChaseEffectClass)
	{
		UE_LOG(LogTemp, Log, TEXT("DoChase"));
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ChaseEffectClass);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
	OwnerPawn->SetHuntLevel(EDeafHuntLevel::Chase);

	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));
}

void UGA_DeafChase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

