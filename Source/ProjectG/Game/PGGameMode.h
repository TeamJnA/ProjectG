// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PGGameMode.generated.h"

class APGPlayerController;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	APGGameMode();

protected:
	virtual void BeginPlay() override;
	// virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	UFUNCTION()
	void HandleMapGenerationComplete();

	UFUNCTION()
	void HandleSpawnComplete();

	UFUNCTION()
	void HandleClientTravel();

	void SpawnAllPlayers();
	void CheckPlayerReady(TWeakObjectPtr<APGPlayerController> WeakPC);

	void SpawnGlobalLightManager();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	bool bManagerSpawned = false;
	bool bDelegateFailed = false;
	int32 ConnectedPlayerCount = 0;
	float SpawnOffset = 0.0f;
};
