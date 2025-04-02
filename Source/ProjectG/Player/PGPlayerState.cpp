// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGPlayerState.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystem/PGAttributeSet.h"

APGPlayerState::APGPlayerState()
{
	NetUpdateFrequency = 100.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPGAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* APGPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPGAttributeSet* APGPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
