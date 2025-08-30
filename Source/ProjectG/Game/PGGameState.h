// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "Net/UnrealNetwork.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayerListUpdatedDelegate);

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

USTRUCT(BlueprintType)
struct FPlayerLobbyInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> AvatarRawData;

	UPROPERTY(BlueprintReadOnly)
	int32 AvatarWidth = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 AvatarHeight = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsHost = false;
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

	// ----- Lobby Player List ---------
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnLobbyPlayerListUpdatedDelegate OnLobbyPlayerListUpdated;

	// Ŭ���̾�Ʈ�� ������ ������ ������ �÷��̾� ���
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LobbyPlayerList, Category = "Lobby")
	TArray<FPlayerLobbyInfo> LobbyPlayerList;

	// only call on server
	void UpdateLobbyPlayerList();

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
	// LobbyPlayerList�� Ŭ���̾�Ʈ�� ������ �� ȣ��� �Լ�
	UFUNCTION()
	void OnRep_LobbyPlayerList();
};
