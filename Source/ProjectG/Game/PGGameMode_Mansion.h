// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/PGGameMode.h"
#include "PGGameMode_Mansion.generated.h"

class APGGhostCharacter;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGGameMode_Mansion : public APGGameMode
{
	GENERATED_BODY()
	
public:
	APGGameMode_Mansion();

	void SpawnGhost(const FTransform& SpawnTransform);
	virtual void SetPlayerReadyToReturnLobby(APlayerState* PlayerState) override;

protected:
	virtual void Logout(AController* Exiting) override;

private:
	void CleanupGeometryCollections();

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<APGGhostCharacter> GhostCharacterClass;
};
