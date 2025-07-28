// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "Net/UnrealNetwork.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);

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

	//bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	//{
	//	Ar << PlayerUniqueId;
	//	Ar << bIsReady;
	//	bOutSuccess = true;
	//	return true;
	//}

	//bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	//{
	//	return FastArrayDeltaSerialize<FPlayerReadyState, FPlayerReadyState>(PlayerReadyStateArray, DeltaParams, *this);

	//}
	//TArray<FPlayerReadyState>& PlayerReadyStateArray = *new TArray<FPlayerReadyState>(); // 더미이지만 필요

};

//template<>
//struct TStructOpsTypeTraits<FPlayerReadyState> : public TStructOpsTypeTraitsBase2<FPlayerReadyState>
//{
//	enum
//	{
//		WithNetSerializer = true,
//		WithNetDeltaSerializer = true
//	};
//};

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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

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
};
