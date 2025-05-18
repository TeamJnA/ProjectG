// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Ability/PGBlindAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "ProjectG/Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

UPGBlindAttributeSet::UPGBlindAttributeSet()
{
}

void UPGBlindAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/// With this macro, OnRep_Stamina() is called on the client whenever the stamina value changes.
	/// COND_None : Replicated to all clients. (ex. health bar should be replicated to all clients so everyone can see health)
	/// COND_OwnerOnly : Replicated to only actors owner.

	DOREPLIFETIME_CONDITION_NOTIFY(UPGBlindAttributeSet, HearingRange, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPGBlindAttributeSet, MaxHearingRange, COND_OwnerOnly, REPNOTIFY_Always);
}

void UPGBlindAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHearingRangeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHearingRange());
	}
}

void UPGBlindAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);


	if (Data.EvaluatedData.Attribute == GetHearingRangeAttribute())
	{
		AActor* Owner = GetOwningActor();
		if (APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(Owner))
		{
			if (APGBlindAIController* AIC = Cast<APGBlindAIController>(OwnerPawn->GetController()))
			{
				AIC->SetHearingRange(HearingRange.GetCurrentValue());
			}
		}
	}

	/* 요런 식으로 접근가능.

	UAbilitySystemComponent* ASC = Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent();
	AActor* ASCOwner = ASC->AbilityActorInfo->OwnerActor.Get();
	ASCOwner->GetActorLocation();*/
}

void UPGBlindAttributeSet::OnRep_HearingRange(const FGameplayAttributeData& OldHearingRange) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGBlindAttributeSet, HearingRange, OldHearingRange);
	
	AActor* Owner = GetOwningActor();
	if (APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(Owner))
	{
		if (APGBlindAIController* AIC = Cast<APGBlindAIController>(OwnerPawn->GetController()))
		{
			AIC->SetHearingRange(HearingRange.GetCurrentValue());
		}
	}
}

void UPGBlindAttributeSet::OnRep_MaxHearingRange(const FGameplayAttributeData& OldMaxHearingRange) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGBlindAttributeSet, MaxHearingRange, OldMaxHearingRange);
}
