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

	void SetPlayerReadyToReturnLobby(const APlayerState* PlayerState);

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

protected:
	virtual void BeginPlay() override;
	// virtual void PostLogin(APlayerController* NewPlayer) override;
	// virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	// virtual void Logout(AController* Exiting) override;

	void CheckAllPlayersArrived(const TSet<TObjectPtr<APlayerState>>& InExpectedPlayers);

	UFUNCTION()
	void HandleMapGenerationComplete();

	void SpawnAllPlayers();
	void SpawnLevelGenerator();
	void SpawnGlobalLightManager();

	TSet<TObjectPtr<APlayerState>> ArrivedPlayers;

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	bool bGamemodeReady = false;
	float SpawnOffset = 0.0f;

	void InitSoundManagerToPlayers();
};
