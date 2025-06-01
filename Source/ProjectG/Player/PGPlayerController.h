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
	void NotifyTravelFailed();

protected:	
	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;

	UFUNCTION(Client, Reliable)
	void Client_PostSeamlessTravel();

	UFUNCTION(Server, Reliable)
	void Server_ReportTravelFailed();
};
