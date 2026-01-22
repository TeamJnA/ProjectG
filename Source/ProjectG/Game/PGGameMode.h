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
class APGGhostCharacter;

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
	
	void RespawnPlayer(AController* DeadPlayerController, const FTransform& SpawnTransform);

	void SpawnGhost(const FTransform& SpawnTransform);

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

	void UpdateSpectatorsTarget(const ACharacter* RevivedCharacter, const APlayerState* RevivedPlayerState);

	UPROPERTY()
	TSet<FUniqueNetIdRepl> ArrivedPlayers;

	UPROPERTY(BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<APGSoundManager> SoundManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<APGGhostCharacter> GhostCharacterClass;

	float SpawnOffset = 0.0f;

	void InitSoundManagerToPlayers();
};
