// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "Interface/SoundManagerInterface.h"
#include "Type/PGGameTypes.h"

#include "PGGameMode.generated.h"

class APGPlayerController;
class APGSoundManager;
class APGLevelGenerator;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGameMode : public AGameMode, public ISoundManagerInterface
{
	GENERATED_BODY()

public:
	APGGameMode();

	void PlayerTravelSuccess(APlayerController* Player);

	virtual void SetPlayerReadyToReturnLobby(APlayerState* PlayerState);

	virtual void HandlePlayerEscaping(ACharacter* EscapingPlayer, EExitPointType ExitPointType = EExitPointType::IronDoor);
	
	void RespawnPlayer(AController* DeadPlayerController, const FTransform& SpawnTransform);

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;
	// virtual void PostLogin(APlayerController* NewPlayer) override;
	// virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void CheckAllPlayersArrived();

	UFUNCTION()
	void HandleMapGenerationComplete();

	void SpawnAllPlayers();
	void SpawnLevelGenerator();
	void SpawnGlobalLightManager();

	void UpdateSpectatorsTarget(const ACharacter* RevivedCharacter, const APlayerState* RevivedPlayerState);

	void InitSoundManagerToPlayers();

public:
	void ProcessSoloLeaveRequest(APGPlayerController* RequestingPC, ECleanupActionType ActionType);
	void RequestSessionDestruction(bool bServerQuit);

protected:
	void PerformSoloLeave(APGPlayerController* TargetPC);
	void RequestServerTravel();
	void CancelAllPendingSoloLeaves();
	void BroadcastCleanupCommand();
	void ExecutePendingAction();

	// Travel 할 Level을 저장할 변수. GameMode의 자식 클래스에서 재지정해서 사용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	FString ReturnTravelURL = TEXT("/Game/ProjectG/Levels/LV_PGLobbyRoom?listen");

private:
	UPROPERTY()
	TMap<APGPlayerController*, ECleanupActionType> PendingSoloLeavers;

	UPROPERTY()
	TMap<APGPlayerController*, FTimerHandle> SoloLeaveTimers;

	UPROPERTY()
	TSet<FUniqueNetIdRepl> ArrivedPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawn", meta = (AllowPrivateAccess = true))
	TArray<FTransform> PlayerSpawnTransforms;

	UPROPERTY(BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<APGSoundManager> SoundManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<APGLevelGenerator> PGLevelGenerator;

	FTimerHandle InGameStartTimerHandle;
	FTimerHandle TravelCheckTimerHandle;

	float TravelCheckElapsed = 0.0f;
	float TravelCheckDeadline = 20.0f;

	ECleanupActionType PendingActionType = ECleanupActionType::None;

	bool bIsMassProcessing = false;
	bool bServerShouldQuit = false;
};
