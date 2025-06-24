// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PGGM_Test.generated.h"

class APGSoundManager;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGGM_Test : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	void InitSoundManagerToPlayers();

	UPROPERTY()
	TObjectPtr<APGSoundManager> SoundManager;

	FTimerHandle TimerHandle;
};
