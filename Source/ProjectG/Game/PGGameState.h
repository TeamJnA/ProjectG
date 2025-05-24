// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClientTravelComplete);

class APGPlayerController;

UCLASS()
class PROJECTG_API APGGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void NotifyMapGenerationComplete();
	void NotifyClientTravelComplete(APGPlayerController* PC);
	void NotifyGameModeReady();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerationComplete OnMapGenerationComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnClientTravelComplete OnClientTravelComplete;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_MapGenerationComplete();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ClientTravelComplete();

private:
	TSet<APlayerController*> ClientTravelCompletedPlayersQueue;
	bool bGameModeReady = false;
};
