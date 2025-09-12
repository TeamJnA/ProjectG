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

	EGameState GetCurrentGameState() const { return CurrentGameState; }
	void SetCurrentGameState(EGameState NewGameState) { CurrentGameState = NewGameState; }
	const TArray<FPlayerInfo>& GetPlayerList() const { return PlayerList; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerationComplete OnMapGenerationComplete;

	// EndGame
	bool IsGameFinished();

	void NotifyGameFinished();

	void SetPlayerReadyStateForReturnLobby(const APlayerState* InPlayerState);

	bool IsAllReadyToReturnLobby() const;

	// ----- Player List ---------
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnPlayerListUpdatedDelegate OnPlayerListUpdated;
	
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
	void Multicast_InitFinalScoreBoardWidget();

	// Ŭ���̾�Ʈ�� ������ ������ ������ �÷��̾� ���
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerList, Category = "Lobby")
	TArray<FPlayerInfo> PlayerList;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	EGameState CurrentGameState;

	// ----- Lobby Player List ---------
	// LobbyPlayerList�� Ŭ���̾�Ʈ�� ������ �� ȣ��� �Լ�
	UFUNCTION()
	void OnRep_PlayerList();
};
