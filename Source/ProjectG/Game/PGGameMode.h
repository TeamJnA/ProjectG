// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "Interface/SoundManagerInterface.h"
#include "Type/PGGameTypes.h"

#include "PGGameMode.generated.h"

class APGPlayerController;
class APGSoundManager;
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

	void HandlePlayerEscaping(ACharacter* EscapingPlayer, EExitPointType ExitPointType = EExitPointType::IronDoor);
	
	void RespawnPlayer(AController* DeadPlayerController, const FTransform& SpawnTransform);

	void SpawnGhost(const FTransform& SpawnTransform);

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

protected:
	virtual void BeginPlay() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
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

	void InitSoundManagerToPlayers();

	UPROPERTY()
	TSet<FUniqueNetIdRepl> ArrivedPlayers;

	UPROPERTY(BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<APGSoundManager> SoundManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<APGGhostCharacter> GhostCharacterClass;

	float SpawnOffset = 0.0f;
};
