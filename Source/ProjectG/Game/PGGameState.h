// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapGenerationComplete);

class APGPlayerController;

UCLASS()
class PROJECTG_API APGGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void NotifyMapGenerationComplete();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMapGenerationComplete OnMapGenerationComplete;

protected:
	void Multicast_MapGenerationComplete();
};
