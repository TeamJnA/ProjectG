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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyToReturnLobbyChanged);

class APGPlayerController;
class APGPlayerCharacter;
class ULevelSequence;
class ULevelSequencePlayer;
class UGameplayEffect;

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

	FORCEINLINE EGameState GetCurrentGameState() const { return CurrentGameState; }
	void SetCurrentGameState(EGameState NewGameState) { CurrentGameState = NewGameState; OnRep_CurrentGameState(); }
	
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

	// ----- Return To Lobby ---------
	FOnReadyToReturnLobbyChanged OnReadyToReturnLobbyChanged;

	void NotifyReadyToReturnLobbyChanged();
	int32 GetReadyToReturnLobbyCount() const;
	void ResetAllReadyToReturnLobby();
	// ----- Return To Lobby ---------

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(ReplicatedUsing = OnRep_CurrentGameState, BlueprintReadOnly, Category = "GameState")
	EGameState CurrentGameState; 
	
	UFUNCTION()
	void OnRep_CurrentGameState();

	UPROPERTY()
	TMap<EExitPointType, TObjectPtr<AActor>> ExitCameraMap;

// Play Seqeunce
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence")
	TSoftObjectPtr<ULevelSequence> LevelSequenceAsset;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayerEnterLevelSequence(int32 NumPlayers);

protected:
	void PlayEnterLevelSeqeunce(int32 NumPlayers);

	UFUNCTION()
	void OnEnterSequenceFinished();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USoundBase>> EnterSequenceFootStepSounds;

	UPROPERTY()
	TObjectPtr<ULevelSequence> LoadedLevelSequence;

	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> EnterSequencePlayer;

// Enemy AI Exploration Waypoint
public: 
	FORCEINLINE const TArray<FVector>& GetExplorationWaypoints() const { return ExplorationWaypoints; }
	void SetExplorationWaypoints(const TArray<FVector>& InWaypoints) { ExplorationWaypoints = InWaypoints; }

	FVector GetExplorationTarget(const FVector& CurrentLocation) const;

	void DrawDebugWaypoints() const;

protected:
	UPROPERTY()
	TArray<FVector> ExplorationWaypoints;

public:
	void StartMaxSanityDecreaseTimer();
	FORCEINLINE int32 GetMaxSanityDecreaseCount() const { return MaxSanityDecreaseCount; }

private:
	void OnMaxSanityDecreaseTick();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySanityBellSequence(int32 BellCount, bool bUseEerieSound);

	FTimerHandle MaxSanityDecreaseTimerHandle;
	FTimerHandle BellSequenceTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	TSubclassOf<UGameplayEffect> DecreaseMaxSanityEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	TObjectPtr<USoundBase> MaxSanityDecreaseBellSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	TObjectPtr<USoundBase> MaxSanityDecreaseEerieBellSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	float MaxSanityDecreaseInterval = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	float MaxSanityDecreasePerTick = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	float BellInterval = 3.0f;

	UPROPERTY(Replicated)
	int32 MaxSanityDecreaseCount = 0;

	int32 RemainingBells = 0;

	bool bCurrentUseEerie = false;
};
