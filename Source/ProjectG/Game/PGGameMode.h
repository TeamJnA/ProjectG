// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "Interface/SoundManagerInterface.h"

#include "PGGameMode.generated.h"

class APGPlayerController;
class APGSoundManager;

// TEST TO REMOVE
class APGBlindCharacter;

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

	void SetPlayerReadyToReturnLobby(APlayerState* PlayerState);

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

protected:
	virtual void BeginPlay() override;
	// virtual void PostLogin(APlayerController* NewPlayer) override;
	// virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	// virtual void Logout(AController* Exiting) override;

	void CheckAllPlayersArrived();

	UFUNCTION()
	void HandleMapGenerationComplete();

	void SpawnAllPlayers();
	void SpawnLevelGenerator();
	void SpawnGlobalLightManager();
	void SpawnEnemy();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	TSet<TObjectPtr<APlayerState>> ExpectedPlayers;
	TSet<TObjectPtr<APlayerState>> ArrivedPlayers;
	FTimerHandle TravelCheckTimer;

	bool bLevelGeneratorSpawned = false;
	bool bManagerSpawned = false;

	bool bGamemodeReady = false;
	float SpawnOffset = 0.0f;

	void InitSoundManagerToPlayers();

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

	//TEST TO REMOVE
	UPROPERTY(EditAnywhere, Category = "Enemy")
	TSubclassOf<APGBlindCharacter> BlindCharacterToSpawnTEST;
};
