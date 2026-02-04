// Fill out your copyright notice in the Description page of Project Settings.

#include "PGAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "PGLogChannels.h"

UPGAttributeSet::UPGAttributeSet()
{
	InitStamina(100.0f);
	InitSanity(100.0f);
}

void UPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/// With this macro, OnRep_Stamina() is called on the client whenever the stamina value changes.
	/// COND_None : Replicated to all clients. (ex. health bar should be replicated to all clients so everyone can see health)
	/// COND_OwnerOnly : Replicated to only actors owner.
	DOREPLIFETIME_CONDITION_NOTIFY(UPGAttributeSet, Stamina, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPGAttributeSet, MaxStamina, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UPGAttributeSet, Sanity, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPGAttributeSet, MaxSanity, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UPGAttributeSet, MovementSpeed, COND_OwnerOnly, REPNOTIFY_Always);
}

void UPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetSanityAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSanity());
	}
}

void UPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		/*
		if (GetStamina() <= 1.0f)
		{
			GetOwningAbilitySystemComponent()->GetAvatarActor();
			GetOwningAbilitySystemComponent()->CancelAbilities();
		}*/
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetSanityAttribute())
	{
		SetSanity(FMath::Clamp(GetSanity(), 0.0f, GetMaxSanity()));
	}
}

void UPGAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	//Inform gameplay ability system about the attribute change
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGAttributeSet, Stamina, OldStamina);
}

void UPGAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGAttributeSet, MaxStamina, OldMaxStamina);
}

void UPGAttributeSet::OnRep_Sanity(const FGameplayAttributeData& OldSanity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGAttributeSet, Sanity, OldSanity);
}

void UPGAttributeSet::OnRep_MaxSanity(const FGameplayAttributeData& OldMaxSanity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGAttributeSet, MaxSanity, OldMaxSanity);
}

void UPGAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGAttributeSet, MovementSpeed, OldMovementSpeed);
}