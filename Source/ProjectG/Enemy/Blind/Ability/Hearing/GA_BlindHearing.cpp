// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Ability/Hearing/GA_BlindHearing.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

UGA_BlindHearing::UGA_BlindHearing()
{
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;


	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.State.Deafened")));
	
	
}

void UGA_BlindHearing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	//UE_LOG(LogTemp, Warning, TEXT("Activate Abiltiy"));

	APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (OwnerPawn)
	{
		if (APGBlindAIController* AIC = Cast<APGBlindAIController>(OwnerPawn->GetController()))
		{
			AIC->SetHearingEnabled(true);
		}
	}



}

void UGA_BlindHearing::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	//UE_LOG(LogTemp, Warning, TEXT("End Abiltiy"));


	APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (OwnerPawn)
	{
		if (APGBlindAIController* AIC = Cast<APGBlindAIController>(OwnerPawn->GetController()))
		{
			AIC->SetHearingEnabled(false);
		}
	}
}

void UGA_BlindHearing::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// Ability 자동 실행
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		const bool bSuccess = ASC->TryActivateAbility(Spec.Handle);
	}
}
