// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameState.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGLobbyGameMode.h"
#include "Player/PGPlayerController.h"
#include "Player/PGPlayerState.h"
#include "Character/PGPlayerCharacter.h"

#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"
#include "Net/NetPushModelHelpers.h"

void APGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGGameState, CurrentGameState);
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

	UE_LOG(LogTemp, Log, TEXT("GS::AddPlayerState: [%s] Player added | %d"), *PlayerState->GetName(), HasAuthority());
	NotifyPlayerArrayUpdated();

	if (HasAuthority())
	{
		RequestUpdateSessionPlayerCount();
	}
}

/*
* 플레이어 퇴장 시 PlayerList 업데이트 
*/
void APGGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	UE_LOG(LogTemp, Log, TEXT("GS::RemovePlayerState: Player removed | %d"), HasAuthority());
	NotifyPlayerArrayUpdated();

	if (HasAuthority())
	{
		RequestUpdateSessionPlayerCount();
	}
}

void APGGameState::RequestUpdateSessionPlayerCount()
{
	const float UpdateDelay = 0.3f;
	GetWorld()->GetTimerManager().SetTimer(
		SessionPlayerCountUpdateTimer,
		this,
		&APGGameState::UpdateSessionPlayerCount_Internal,
		UpdateDelay,
		false
	);
}

void APGGameState::UpdateSessionPlayerCount_Internal()
{
	if (!HasAuthority())
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		return;
	}
	GI->UpdateSessionPlayerCount(PlayerArray.Num());
}

void APGGameState::NotifyPlayerArrayUpdated()
{
	UE_LOG(LogTemp, Log, TEXT("GS::UpdatePlayerArrayVersion: Player array updated | %d"), HasAuthority());
	OnPlayerArrayChanged.Broadcast();
}

/*
* 레벨 생성 완료 후 LevelGenerator에서 호출
*/
void APGGameState::NotifyMapGenerationComplete()
{
	Multicast_MapGenerationComplete();
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

	FTimerHandle FinalScoreBoardTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(FinalScoreBoardTimerHandle, this, &APGGameState::Multicast_InitFinalScoreBoardWidget, 2.0f, false);
}

/*
* 게임이 종료 상태가 되었는지 확인
* 모든 플레이어가 종료 상태인 경우 true
*/
bool APGGameState::IsGameFinished() const
{
	if (PlayerArray.IsEmpty())
	{
		return false;
	}

	for (const APlayerState* PS : PlayerArray)
	{
		if (const APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			if (!PGPS->HasFinishedGame())
			{
				return false;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("APGGameState::IsGameFinished: All players have finished the game."));
	return true;
}

bool APGGameState::IsAllReadyToReturnLobby() const
{
	if (!HasAuthority())
	{
		return false;
	}

	if (PlayerArray.IsEmpty())
	{
		return false;
	}

	for (const APlayerState* PS : PlayerArray)
	{
		if (const APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			if (!PGPS->IsReadyToReturnLobby())
			{
				return false;
			}
		}
	}

	return true;
}

void APGGameState::NotifyPlayerFinished(APlayerState* FinishedPlayerState)
{
	if (!HasAuthority() || !FinishedPlayerState)
	{
		return;
	}
	HandlePlayerFinished(FinishedPlayerState);
}

/*
* 플레이어 종료 처리 구현부
* 플레이어를 종료 상태로 설정 (!사망 && 종료 -> 탈출 상태)
* 플레이어 탈출 후 모든 플레이어가 종료 상태인지 확인
* 종료 상태인 경우 게임을 종료 상태로 변경
* 아직 플레이 중인 플레이어가 있는 경우 종료 상태인 플레이어들의 상태를 디스플레이하는 ScoreBoardWidget Init
*/
void APGGameState::HandlePlayerFinished(APlayerState* FinishedPlayerState)
{
	APGPlayerState* PS = Cast<APGPlayerState>(FinishedPlayerState);
	if (PS && !PS->HasFinishedGame())
	{
		PS->SetHasFinishedGame(true);

		APGPlayerCharacter* FinishedPlayerCharacter = Cast<APGPlayerCharacter>(PS->GetPawn());

		if (IsGameFinished())
		{
			SetCurrentGameState(EGameState::EndGame);
			NotifyGameFinished();
		}
		else
		{
			if (FinishedPlayerCharacter)
			{
				FinishedPlayerCharacter->Client_InitScoreBoardWidget();
			}
		}
	}
}