// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClientTravel);

UCLASS()
class PROJECTG_API APGGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void NotifyMapGenerationComplete();
	void NotifySpawnComplete();
	void NotifyClientTravel();

	UFUNCTION()
	void NotifyClientReady(APGPlayerController* PC);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerationComplete OnMapGenerationComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpawnComplete OnSpawnComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnClientTravel OnClientTravel;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_MapGenerationComplete();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnComplete();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ClientTravel();

private:
	TSet<APlayerController*> ReadyPlayers;
};
