// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGPlayerState.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Net/UnrealNetwork.h"

APGPlayerState::APGPlayerState()
{
	SetNetUpdateFrequency(100.0f);

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

void APGPlayerState::ServerSetReady_Implementation(bool bReady)
{
	bIsReady = bReady;
	OnRep_IsReady();
}

void APGPlayerState::OnRep_IsReady()
{
}

void APGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGPlayerState, bIsReady);
}
