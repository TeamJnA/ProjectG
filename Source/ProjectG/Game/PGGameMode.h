// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "Interface/SoundManagerInterface.h"

#include "PGGameMode.generated.h"

class APGPlayerController;
class APGSoundManager;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGameMode : public AGameMode, public ISoundManagerInterface
{
	GENERATED_BODY()

public:
	APGGameMode();

	void SetIsTravelFailedExist();

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

protected:
	virtual void BeginPlay() override;
	// virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION()
	void HandleMapGenerationComplete();

	void PostTravel();

	void SpawnAllPlayers();
	void SpawnLevelGenerator();
	void SpawnGlobalLightManager();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	bool bLevelGeneratorSpawned = false;
	bool bManagerSpawned = false;

	bool bGamemodeReady = false;
	int32 ConnectedPlayerCount = 0;
	float SpawnOffset = 0.0f;

	int32 ExpectedPlayerCount = 0;
	TSet<APlayerController*> ClientTravelCompletedPlayersSet;
	bool bIsTravelFailedExist = false;
	FTimerHandle TravelCheckTimer;

	void InitSoundManagerToPlayers();

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;
};
