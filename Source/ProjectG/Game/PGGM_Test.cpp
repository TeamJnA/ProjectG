// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGM_Test.h"
#include "Character/PGPlayerCharacter.h"
#include "Sound/PGSoundManager.h"

APGSoundManager* APGGM_Test::GetSoundManager()
{
	return SoundManager;
}

void APGGM_Test::BeginPlay()
{
	// Spawn sound manager on the lever.
	SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
	if (SoundManager) {
		UE_LOG(LogTemp, Log, TEXT("Sound manager spawn complete."));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APGGM_Test::InitSoundManagerToPlayers, 4.0f, false);
}

void APGGM_Test::InitSoundManagerToPlayers()
{
	// Set the character's SoundManager pointer to the globally spawned soundmanager instance.
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* PC = it->Get();
		if (!PC) continue;

		APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(PC->GetPawn());
		if (PGPC)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s"), *PGPC->GetName());
			PGPC->InitSoundManager(SoundManager);
		}
	}
}