// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PGLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	APGLobbyGameMode();

	void StartGame();

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> PlayerPawnClass;

};
