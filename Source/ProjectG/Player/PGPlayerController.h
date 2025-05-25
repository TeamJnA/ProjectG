// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void Client_InitiateTravelTimer();

protected:	
	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;
};
