// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGPlayerState.h"

#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerController.h"
#include "Player/PGLobbyPlayerController.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/PGSpectatorPawn.h"

#include "Net/UnrealNetwork.h"

APGPlayerState::APGPlayerState()
{
	bAlwaysRelevant = true;
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
	DOREPLIFETIME(APGPlayerState, ExitPoint);
	DOREPLIFETIME(APGPlayerState, bIsSpectating);
}

void APGPlayerState::OnRep_PlayerStateUpdated()
{
	if (IsInactive()) 
	{
		return;
	}

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (GS && !GS->PlayerArray.Contains(this))
	{
		return;
	}

	FString NetModeStr = (GetNetMode() == NM_Client) ? TEXT("Client") : TEXT("Server");
	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] OnRep_PlayerStateUpdated triggered! Target PS: %s"), *NetModeStr, *GetPlayerName());

	APlayerController* LocalPC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!LocalPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] LocalPC not found! Retrying in 0.1s..."), *NetModeStr);
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerState::OnRep_PlayerStateUpdated, 0.1f, false);
		return;
	}

	APGPlayerController* InGamePC = Cast<APGPlayerController>(LocalPC);
	APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(LocalPC);
	if (!InGamePC && !LobbyPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] Can not define PC class! Retrying in 0.1s..."), *NetModeStr);
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerState::OnRep_PlayerStateUpdated, 0.1f, false);
		return;
	}

	OnPlayerStateUpdated.Broadcast();

	//if (LocalPC->PlayerState != this)
	if (!IsMyPlayerState())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] Judgment: This is 'Remote' state change. (Target: %s) Updating 3D/2D settings and local voice channels."), *NetModeStr, *GetPlayerName());
		UpdateVoiceSettings();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] Judgment: This is 'Local' state change. Updating voice channel (Mute) rules only."), *NetModeStr);
	}

	if (InGamePC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] RefreshVoiceChannel PGPC"));
		InGamePC->RefreshVoiceChannel();
	}
	else if (LobbyPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] RefreshVoiceChannel LobbyPC"));
		LobbyPC->RefreshVoiceChannel();
	}
}

bool APGPlayerState::IsMyPlayerState() const
{
	if (APlayerController* PC = GetPlayerController())
	{
		return PC->IsLocalController();
	}

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC)
	{
		if (ULocalPlayer* LP = LocalPC->GetLocalPlayer())
		{
			FUniqueNetIdRepl LocalId = LP->GetPreferredUniqueNetId();
			FUniqueNetIdRepl MyId = GetUniqueId();

			if (LocalId.IsValid() && MyId.IsValid() && LocalId == MyId)
			{
				return true;
			}
		}
	}

	return false;
}

void APGPlayerState::UpdateVoiceSettings()
{
	APawn* CurrentPawn = GetPawn();
	if (!CurrentPawn) 
	{
		return;
	}

	if (APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(CurrentPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceDebug] Update PGCharacter VOIP Settings"));
		PGCharacter->UpdateVoipSettings();
	}
	else if (APGSpectatorPawn* PGSpectator = Cast<APGSpectatorPawn>(CurrentPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceDebug] Update Spectator VOIP Settings"));
		PGSpectator->UpdateVoipSettings();
	}
}
