// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "Interface/SoundManagerInterface.h"
#include "Type/PGGameTypes.h"

#include "PGLobbyGameMode.generated.h"

class APGSoundManager;
class APGLobbyPlayerController;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGLobbyGameMode : public AGameMode, public ISoundManagerInterface
{
	GENERATED_BODY()

public:
	APGLobbyGameMode();

	void StartGame();
	void SpawnAndPossessPlayer(APlayerController* NewPlayer);

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

protected:
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	void ProcessSoloLeaveRequest(APGLobbyPlayerController* RequestingPC, ECleanupActionType ActionType);
	void RequestSessionDestruction(bool bServerQuit);

private:
	void PerformSoloLeave(APGLobbyPlayerController* TargetPC);
	void RequestServerTravel();
	void CancelAllPendingSoloLeaves();
	void BroadcastCleanupCommand();
	void ExecutePendingAction();

	UPROPERTY()
	TMap<APGLobbyPlayerController*, ECleanupActionType> PendingSoloLeavers;

	UPROPERTY()
	TMap<APGLobbyPlayerController*, FTimerHandle> SoloLeaveTimers;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

	ECleanupActionType PendingActionType = ECleanupActionType::None;

	bool bIsMassProcessing = false;
	bool bServerShouldQuit = false;
};
