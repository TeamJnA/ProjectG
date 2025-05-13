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
	//WIP
	UFUNCTION(Client, Reliable)
	void Client_CheckLevelSync();

	UFUNCTION(Server, Reliable)
	void Server_RequestSpawnComplete();

	UFUNCTION(Server, Reliable)
	void Server_ReportClientReady();

protected:	
	virtual void BeginPlay() override;

private:
	FTimerHandle TravelCheckHandle;
	bool bSpawnCompleteSent = false;
};
