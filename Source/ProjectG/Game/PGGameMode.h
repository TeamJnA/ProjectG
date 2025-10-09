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

	void HandlePlayerEscaping(ACharacter* EscapingPlayer);

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

	UPROPERTY()
	TSet<FUniqueNetIdRepl> ArrivedPlayers;

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	float SpawnOffset = 0.0f;

	void InitSoundManagerToPlayers();
};
