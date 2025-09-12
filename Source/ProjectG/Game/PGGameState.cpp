// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGLobbyGameMode.h"
#include "Player/PGPlayerController.h"
#include "Player/PGPlayerState.h"

#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"
#include "Net/NetPushModelHelpers.h"

void APGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGGameState, CurrentGameState);
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

/*
* 플레이어 입장 시 PlayerList 업데이트
*/
void APGGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	UE_LOG(LogTemp, Log, TEXT("GS::AddPlayerState: Player added"));	
	UpdatePlayerList();
}

/*
* 플레이어 퇴장 시 PlayerList 업데이트 
*/
void APGGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	UE_LOG(LogTemp, Log, TEXT("GS::RemovePlayerState: Player removed"));
	UpdatePlayerList();
}

/*
* 플레이어 업데이트
* 업데이트는 서버에서만 수행
* 현재 PlayerArray의 PlayerState에 접근해서 필요한 정보 사용 
*/
void APGGameState::UpdatePlayerList()
{
	if (!HasAuthority()) 
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

/*
* PlayerList 업데이트 시 클라이언트 호출
*/
void APGGameState::OnRep_PlayerList()
{
	// update client UI
	OnPlayerListUpdated.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("GS::OnRep_LobbyPlayerList: New player list replicated to client. Broadcasting update."));
}

void APGGameState::Multicast_MapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameState: Multicast_MapGenerationComplete | HasAuthority = %d"), HasAuthority());
	OnMapGenerationComplete.Broadcast();
}

/*
* 게임 종료 후 각 플레이어에 전체 플레이어 상태를 디스플레이하는 FinalScoreBoardWidget Init 명령
*/
void APGGameState::Multicast_InitFinalScoreBoardWidget_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GS::Multicast_InitFinalScoreBoardWidget_Implementation: HasAuthority = %d"), HasAuthority());

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

/*
* 레벨 생성 완료 후 LevelGenerator에서 호출
*/
void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
}

/*
* 게임 종료 후 처리 요청 시 호출
* GameState 내부 MulticastRPC 함수 호출
*/
void APGGameState::NotifyGameFinished()
{
	if (!HasAuthority()) 
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("GS::NotifyGameFinished: called [%s] | HasAuthority = %d"), *GetNameSafe(this), HasAuthority());

	Multicast_InitFinalScoreBoardWidget();
}

/*
* PlayerList의 플레이어 상태를 종료 상태로 설정
*/
void APGGameState::MarkPlayerAsFinished(APlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState)
	{
		return;
	}

	const FUniqueNetIdRepl& FinishedPlayerId = PlayerState->GetUniqueId();
	for (FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (PlayerInfo.PlayerNetId == FinishedPlayerId)
		{
			PlayerInfo.bHasFinishedGame = true;
			UE_LOG(LogTemp, Log, TEXT("GameState: Marked player %s as finished."), *PlayerInfo.PlayerName);
			break;
		}
	}

	// update server UI
	OnRep_PlayerList();
}

/*
* PlayerList의 플레이어 상태를 사망 상태로 설정
*/
void APGGameState::MarkPlayerAsDead(APlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState)
	{
		return;
	}

	const FUniqueNetIdRepl& DeadPlayerId = PlayerState->GetUniqueId();
	for (FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (PlayerInfo.PlayerNetId == DeadPlayerId)
		{
			PlayerInfo.bIsDead = true;
			UE_LOG(LogTemp, Log, TEXT("GameState: Marked player %s as Dead."), *PlayerInfo.PlayerName);
			break;
		}
	}

	// update server UI
	OnRep_PlayerList();
}

/*
* 게임이 종료 상태가 되었는지 확인
* 모든 플레이어가 종료 상태인 경우 true
*/
bool APGGameState::IsGameFinished()
{
	if (PlayerList.IsEmpty())
	{
		return false;
	}

	for (const FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (!PlayerInfo.bHasFinishedGame)
		{
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("APGGameState::IsGameFinished: All players have finished the game."));
	return true;
}

/*
* 요청한 PlayerList의 bIsReadyToReturnLobby를 true로 설정
*/
void APGGameState::SetPlayerReadyStateForReturnLobby(const APlayerState* InPlayerState)
{
	if (!HasAuthority())
	{
		return;
	}

	if (InPlayerState)
	{
		const FUniqueNetIdRepl& TargetPlayerId = InPlayerState->GetUniqueId();
		for (FPlayerInfo& PlayerInfo : PlayerList)
		{
			if (PlayerInfo.PlayerNetId == TargetPlayerId)
			{
				PlayerInfo.bIsReadyToReturnLobby = true;
				break;
			}
		}
	}
}

bool APGGameState::IsAllReadyToReturnLobby() const
{
	if (!HasAuthority())
	{
		return false;
	}

	if (PlayerList.IsEmpty())
	{
		return false;
	}

	for (const FPlayerInfo& PlayerInfo : PlayerList)
	{
		if (!PlayerInfo.bIsReadyToReturnLobby)
		{
			return false;
		}
	}

	return true;
}