// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PGGameMode.generated.h"

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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& OldController) override;

	UFUNCTION()
	void HandleMapGenerationComplete();

	UFUNCTION()
	void HandleSpawnComplete();

	void SpawnAllPlayers();
	void SpawnGlobalLightManager();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

	bool bManagerSpawned = false;
	bool bDelegateFailed = false;
	int32 ConnectedPlayerCount = 0;
	float SpawnOffset = 0.0f;
};
