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

void APGPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("PS::BeginPlay: [%s] PlayerState Begin"), *GetPlayerName());
	OnPlayerStateUpdated.Broadcast();
}

void APGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGPlayerState, bIsHost);
	DOREPLIFETIME(APGPlayerState, bHasFinishedGame);
	DOREPLIFETIME(APGPlayerState, bIsDead);
	DOREPLIFETIME(APGPlayerState, bIsReadyToReturnLobby);
	DOREPLIFETIME(APGPlayerState, bIsEscaping);
}

void APGPlayerState::OnRep_PlayerStateUpdated()
{
	UE_LOG(LogTemp, Log, TEXT("PS::OnRep_PlayerStateUpdated: [%s] PlayerState updated"), *GetPlayerName());
	OnPlayerStateUpdated.Broadcast();
}
