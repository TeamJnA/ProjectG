// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGPlayerState.h"

#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerController.h"

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

	//APlayerController* PC = GetPlayerController();
	//if (PC && !PC->IsLocalController())
	//{
	//	InitVoiceTalker();
	//}
}

void APGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGPlayerState, bIsHost);
	DOREPLIFETIME(APGPlayerState, bHasFinishedGame);
	DOREPLIFETIME(APGPlayerState, bIsDead);
	DOREPLIFETIME(APGPlayerState, bIsReadyToReturnLobby);
	DOREPLIFETIME(APGPlayerState, bIsEscaping);
	DOREPLIFETIME(APGPlayerState, ExitPoint);
	DOREPLIFETIME(APGPlayerState, bIsSpectating);
}

void APGPlayerState::OnRep_PlayerStateUpdated()
{
	UE_LOG(LogTemp, Log, TEXT("PS::OnRep_PlayerStateUpdated: [%s] PlayerState updated"), *GetPlayerName());
	OnPlayerStateUpdated.Broadcast();

	if (APGPlayerController* PC = Cast<APGPlayerController>(GetPlayerController()))
	{
		if (PC && PC->IsLocalController())
		{
			UpdateVoiceSettings();     // 내 목소리 설정 (3D/2D)
			PC->RefreshVoiceChannel(); // 듣는 대상 설정 (Mute/Unmute)
		}
	}
}

void APGPlayerState::InitVoiceTalker()
{
	if (VoipTalker)
	{
		return;
	}

	VoipTalker = UVOIPTalker::CreateTalkerForPlayer(this);
	if (VoipTalker)
	{
		UpdateVoiceSettings();
	}
}

void APGPlayerState::UpdateVoiceSettings()
{
	if (!VoipTalker)
	{
		VoipTalker = UVOIPTalker::CreateTalkerForPlayer(this);
	}

	if (!VoipTalker)
	{
		return;	
	}
	
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	bool bIsGameEnd = GS && GS->GetCurrentGameState() == EGameState::EndGame;

	// FinalScoreBoard, 관전 상태의 음성: 2D
	if (bIsGameEnd || bIsSpectating)
	{
		VoipTalker->Settings.AttenuationSettings = nullptr;
		VoipTalker->Settings.ComponentToAttachTo = nullptr;
	}
	// 인게임 플레이어 음성: 3D
	else if (!bHasFinishedGame)
	{
		VoipTalker->Settings.AttenuationSettings = VoiceAttenuationAsset;
		if (GetPawn()) 
		{
			VoipTalker->Settings.ComponentToAttachTo = GetPawn()->GetRootComponent();
		}
	}
}