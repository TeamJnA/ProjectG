// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGLobbyGameMode.h"
#include "Player/PGPlayerController.h"
#include "Player/PGPlayerState.h"

#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"
#include "Net/NetPushModelHelpers.h"

void APGGameState::SetPlayerReadyStateForReturnLobby(APlayerState* _PlayerState, bool _bIsReady)
{
	if (!HasAuthority()) 
	{
		return;
	}

	if (_PlayerState)
	{
		FString PlayerId = _PlayerState->GetPlayerName();
		bool bFound = false;
		for (FPlayerReadyState& State : PlayerReadyStates)
		{
			if (State.PlayerUniqueId == PlayerId)
			{
				if (State.bIsReady != _bIsReady)
				{
					State.bIsReady = _bIsReady;
				}
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			PlayerReadyStates.Add(FPlayerReadyState(PlayerId, _bIsReady));
		}

		// for debug
		// for server
		OnRep_PlayerReadyStates();
	}

}

void APGGameState::OnRep_PlayerReadyStates()
{
	UE_LOG(LogTemp, Log, TEXT("GS::OnRep_PlayerReadyStates: PlayerReadyStates replicated. Current states:"));
	for (const FPlayerReadyState& State : PlayerReadyStates)
	{
		UE_LOG(LogTemp, Log, TEXT(" Players: %s, ReadyToLobby: %s"), *State.PlayerUniqueId, State.bIsReady ? TEXT("True") : TEXT("False"));
	}
}

bool APGGameState::IsAllReadyToReturnLobby() const
{
	if (!HasAuthority())
	{
		return false;
	}

	for (APlayerState* PS : PlayerArray)
	{
		bool bPlayerFoundAndReady = false;
		FString PlayerID = PS->GetPlayerName();

		for (const FPlayerReadyState& State : PlayerReadyStates)
		{
			if (State.PlayerUniqueId == PlayerID)
			{
				if (State.bIsReady)
				{
					bPlayerFoundAndReady = true;
				}
				break;
			}
		}

		if (!bPlayerFoundAndReady)
		{
			return false;
		}
	}

	return true;
}

void APGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGGameState, CurrentGameState);
	DOREPLIFETIME(APGGameState, FinishedPlayersCount);
	DOREPLIFETIME(APGGameState, PlayerReadyStates);
	DOREPLIFETIME(APGGameState, PlayerList);
}

void APGGameState::BeginPlay()
{
	Super::BeginPlay();
	
	// load saved game state
	if (HasAuthority())
	{
		UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());
		if (GI)
		{
			CurrentGameState = GI->LoadGameStateOnTravel();
			//UE_LOG(LogTemp, Log, TEXT("GS::BeginPlay: Load game state from GI: %s"), *UEnum::GetValueAsString(TEXT("EGameState"), CurrentGameState));
		}
	}
}

void APGGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	UE_LOG(LogTemp, Log, TEXT("GS::AddPlayerState: Player added"));
	// PlayerArray가 변경되었으므로, 복제할 목록을 갱신
	UpdatePlayerList();
}

void APGGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	UE_LOG(LogTemp, Log, TEXT("GS::RemovePlayerState: Player removed"));
	// PlayerArray가 변경되었으므로, 복제할 목록을 갱신
	UpdatePlayerList();
}

void APGGameState::UpdatePlayerList()
{
	// 서버에서만 PlayerList 업데이트 (PlayerList는 replicated라 자동으로 업데이트)
	if (!HasAuthority()) 
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		return;
	}

	PlayerList.Empty();

	for (APlayerState* PS : PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			FPlayerInfo Info;
			Info.PlayerName = PGPS->GetPlayerName();
			Info.bIsHost = PGPS->IsHost();

			const FUniqueNetIdRepl& UniqueIdRepl = PGPS->GetUniqueId();
			if (UniqueIdRepl.IsValid())
			{
				Info.PlayerNetId = UniqueIdRepl;
			}

			PlayerList.Add(Info);
		}
	}

	// update server UI
	OnRep_PlayerList();
}

void APGGameState::OnRep_PlayerList()
{
	// update client UI
	OnPlayerListUpdated.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("GS::OnRep_LobbyPlayerList: New player list replicated to client. Broadcasting update."));
}

void APGGameState::Multicast_MapGenerationComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("GameState: Multicast_MapGenerationComplete | HasAuthority = %d"), HasAuthority());
	OnMapGenerationComplete.Broadcast();
}

void APGGameState::MC_InitFinalScoreBoardWidget_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GS::MC_InitFinalScoreBoardWidget_Implementation: HasAuthority = %d"), HasAuthority());

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		APGPlayerController* PGPC = Cast<APGPlayerController>(PC);
		if (PGPC)
		{
			PGPC->InitFinalScoreBoardWidget();
		}
	}
}

void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
}

void APGGameState::NotifyGameFinished()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("GS::NotifyGameFinished: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());

	MC_InitFinalScoreBoardWidget();
}

void APGGameState::IncreaseFinishedPlayersCount()
{
	FinishedPlayersCount++;
	UE_LOG(LogTemp, Warning, TEXT("APGGameState::IncreaseFinishedPlayersCount: called [%s], FinishedPlayerCount: %d | HasAuthority = %d"), *GetNameSafe(this), FinishedPlayersCount, HasAuthority());

}

bool APGGameState::IsGameFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("APGGameState::IsGameFinished: called [%s], FinishedPlayerCount: %d, PlayerCount: %d | HasAuthority = %d"), *GetNameSafe(this), FinishedPlayersCount, PlayerArray.Num(), HasAuthority());

	return FinishedPlayersCount >= PlayerArray.Num();
}
