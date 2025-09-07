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

UCLASS()
class PROJECTG_API APGGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void NotifyMapGenerationComplete();

	EGameState GetCurrentGameState() { return CurrentGameState; }
	void SetCurrentGameState(EGameState NewGameState) { CurrentGameState = NewGameState; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerationComplete OnMapGenerationComplete;

	// EndGame
	bool IsGameFinished();

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

	void MarkPlayerAsFinished(APlayerState* PlayerState);
	void MarkPlayerAsDead(APlayerState* PlayerState);
	// ----- Player List ---------

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ----- Player List ---------
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	// ----- Player List ---------

	void Multicast_MapGenerationComplete();

	UFUNCTION(NetMulticast, Reliable)
	void MC_InitFinalScoreBoardWidget();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	EGameState CurrentGameState;

	// ----- Lobby Player List ---------
	// LobbyPlayerList가 클라이언트에 복제될 때 호출될 함수
	UFUNCTION()
	void OnRep_PlayerList();
};
