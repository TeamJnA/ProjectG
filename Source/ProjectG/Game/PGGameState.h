// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "Net/UnrealNetwork.h"
#include "Type/CharacterTypes.h"

#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListUpdatedDelegate);

class APGPlayerController;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	MainMenu,
	Lobby,
	InGame,
	EndGame
};

USTRUCT()
struct FPlayerReadyState
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerUniqueId;

	UPROPERTY()
	bool bIsReady;

	FPlayerReadyState() : bIsReady(false) {}
	FPlayerReadyState(FString _PlayerId, bool _IsReady) : PlayerUniqueId(_PlayerId), bIsReady(_IsReady) {}

	bool operator==(const FPlayerReadyState& Other) const
	{
		return PlayerUniqueId == Other.PlayerUniqueId;
	}
};

UCLASS()
class PROJECTG_API APGGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void NotifyMapGenerationComplete();
	void NotifyStartTravel();

	EGameState GetCurrentGameState() { return CurrentGameState; }
	void SetCurrentGameState(EGameState NewGameState) { CurrentGameState = NewGameState; }

	int32 GetFinishedPlayersCount() { return FinishedPlayersCount; }
	void IncreaseFinishedPlayersCount();
	bool IsGameFinished();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerationComplete OnMapGenerationComplete;

	// EndGame
	void NotifyGameFinished();

	void SetPlayerReadyStateForReturnLobby(APlayerState* _PlayerState, bool _bIsReady);

	bool IsAllReadyToReturnLobby() const;

	// ----- Player List ---------
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnPlayerListUpdatedDelegate OnPlayerListUpdated;

	// 클라이언트의 위젯이 접근할 복제된 플레이어 목록
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerList, Category = "Lobby")
	TArray<FPlayerInfo> PlayerList;
	
	// only call on server
	void UpdatePlayerList();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	void Multicast_MapGenerationComplete();

	UFUNCTION(NetMulticast, Reliable)
	void MC_InitiateTravelTimer();

	UFUNCTION(NetMulticast, Reliable)
	void MC_InitFinalScoreBoardWidget();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	EGameState CurrentGameState;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	int32 FinishedPlayersCount;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerReadyStates)
	TArray<FPlayerReadyState> PlayerReadyStates;

	UFUNCTION()
	void OnRep_PlayerReadyStates();

	// ----- Lobby Player List ---------
	// LobbyPlayerList가 클라이언트에 복제될 때 호출될 함수
	UFUNCTION()
	void OnRep_PlayerList();
};
