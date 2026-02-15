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

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

public:
	virtual void Logout(AController* Exiting) override;

	void ProcessSoloLeaveRequest(APGLobbyPlayerController* RequestingPC, ECleanupActionType ActionType);

	void RequestSessionDestruction(bool bServerQuit);

	void OnPlayerCleanupFinished(APlayerController* PC);

private:
	void BroadcastCleanupCommand();
	void ExecutePendingAction();
	void BroadcastRestartVoice();

	bool bIsProcessingAction = false;
	APGLobbyPlayerController* PendingLeaverPC = nullptr;
	ECleanupActionType PendingActionType = ECleanupActionType::None;

	bool bIsMassTravel = false;
	bool bServerShouldQuit = false;

	int32 ReadyPlayerCount = 0;
	int32 TotalPlayerCount = 0;
	FTimerHandle TimeoutTimerHandle;
};
