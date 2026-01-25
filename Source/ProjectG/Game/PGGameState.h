// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "Net/UnrealNetwork.h"
#include "Type/CharacterTypes.h"
#include "Type/PGGameTypes.h"

#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerArrayChangedDelegate);

class APGPlayerController;
class APGPlayerCharacter;

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
	
	FOnMapGenerationComplete OnMapGenerationComplete;

	// Game State Process
	bool IsGameFinished() const;
	void NotifyGameFinished();
	bool IsAllReadyToReturnLobby() const;
	void NotifyPlayerFinished(APlayerState* FinishedPlayerState);


	// ----- Player List ---------
	FOnPlayerArrayChangedDelegate OnPlayerArrayChanged;
	
	void NotifyPlayerArrayUpdated();
	// ----- Player List ---------

	FORCEINLINE void RegisterExitCamera(EExitPointType Type, AActor* CameraActor)
	{
		ExitCameraMap.Add(Type, CameraActor);
	}

	FORCEINLINE AActor* GetExitCameraByEnum(EExitPointType Type) const
	{
		if (const TObjectPtr<AActor>* FoundCamera = ExitCameraMap.Find(Type))
		{
			return FoundCamera->Get();
		}
		return nullptr;
	}

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ----- Player List ---------
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	FTimerHandle SessionPlayerCountUpdateTimer;
	
	void RequestUpdateSessionPlayerCount();
	void UpdateSessionPlayerCount_Internal();
	// ----- Player List ---------

	void Multicast_MapGenerationComplete();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InitFinalScoreBoardWidget();

	void HandlePlayerFinished(APlayerState* FinishedPlayerState);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	EGameState CurrentGameState;

	UPROPERTY()
	TMap<EExitPointType, TObjectPtr<AActor>> ExitCameraMap;
};
