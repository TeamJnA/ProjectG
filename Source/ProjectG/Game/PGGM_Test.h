// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Interface/SoundManagerInterface.h"

#include "PGGM_Test.generated.h"

class APGSoundManager;
class APGBlindCharacter;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGGM_Test : public AGameModeBase, public ISoundManagerInterface
{
	GENERATED_BODY()
	
public:
	APGGM_Test();

	// ISoundManagerInterface~
	virtual APGSoundManager* GetSoundManager() override;
	// ~ISoundManagerInterface

	UFUNCTION(BlueprintCallable)
	void TEST_MakeGroundSpawnMonsterAndBuildNavMesh(FVector MonsterSpawnLocation, FVector GroundSpawnLocation, FVector GroundScale);

	// Test spawn 
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<AActor> Ground;

	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<APGBlindCharacter> EnemyCharacter;

protected:
	virtual void BeginPlay() override;

	void InitSoundManagerToPlayers();

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

	FTimerHandle TimerHandle;
};
