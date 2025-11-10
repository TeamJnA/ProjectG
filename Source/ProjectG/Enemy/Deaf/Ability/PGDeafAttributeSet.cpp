// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Deaf/Ability/PGDeafAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "ProjectG/Enemy/Deaf/Character/PGDeafCharacter.h"
#include "Enemy/Deaf/AI/Controllers/PGDeafAIController.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UPGDeafAttributeSet::UPGDeafAttributeSet()
{
}

void UPGDeafAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/// With this macro, OnRep_Stamina() is called on the client whenever the stamina value changes.
	/// COND_None : Replicated to all clients. (ex. health bar should be replicated to all clients so everyone can see health)
	/// COND_OwnerOnly : Replicated to only actors owner.

	DOREPLIFETIME_CONDITION_NOTIFY(UPGDeafAttributeSet, SightRadius, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPGDeafAttributeSet, MaxSightRadius, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPGDeafAttributeSet, SightAngle, COND_OwnerOnly, REPNOTIFY_Always);
}

void UPGDeafAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetSightRadiusAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSightRadius());
	}

	if (Attribute == GetSightAngleAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 180.f);
	}
}

void UPGDeafAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);


	if (Data.EvaluatedData.Attribute == GetSightRadiusAttribute())
	{
		AActor* Owner = GetOwningActor();
		if (APGDeafCharacter* OwnerPawn = Cast<APGDeafCharacter>(Owner))
		{
			if (APGDeafAIController* AIC = Cast<APGDeafAIController>(OwnerPawn->GetController()))
			{
				AIC->SetSightRadius(SightRadius.GetCurrentValue());
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetSightAngleAttribute())
	{
		AActor* Owner = GetOwningActor();
		if (APGDeafCharacter* OwnerPawn = Cast<APGDeafCharacter>(Owner))
		{
			if (APGDeafAIController* AIC = Cast<APGDeafAIController>(OwnerPawn->GetController()))
			{
				AIC->SetSightAngle(SightAngle.GetCurrentValue());
			}
		}
	}
}

void UPGDeafAttributeSet::OnRep_SightRadius(const FGameplayAttributeData& OldSightRadius) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGDeafAttributeSet, SightRadius, OldSightRadius);

	AActor* Owner = GetOwningActor();
	if (APGDeafCharacter* OwnerPawn = Cast<APGDeafCharacter>(Owner))
	{
		if (APGDeafAIController* AIC = Cast<APGDeafAIController>(OwnerPawn->GetController()))
		{
			AIC->SetSightRadius(SightRadius.GetCurrentValue());
		}
	}
}

void UPGDeafAttributeSet::OnRep_MaxSightRadius(const FGameplayAttributeData& OldMaxSightRadius) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGDeafAttributeSet, MaxSightRadius, OldMaxSightRadius);
}

void UPGDeafAttributeSet::OnRep_SightAngle(const FGameplayAttributeData& OldSightAngle) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGDeafAttributeSet, SightAngle, OldSightAngle);

	AActor* Owner = GetOwningActor();
	if (APGDeafCharacter* OwnerPawn = Cast<APGDeafCharacter>(Owner))
	{
		if (APGDeafAIController* AIC = Cast<APGDeafAIController>(OwnerPawn->GetController()))
		{
			AIC->SetSightAngle(SightAngle.GetCurrentValue());
		}
	}
}
