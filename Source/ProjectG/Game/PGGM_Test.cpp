// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGM_Test.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/PGCharacterBase.h"
#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"

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


	// Enemy 찾아서 InitSoundManager하고 Test 할 것
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APGCharacterBase::StaticClass(), AllCharacters);

	for (AActor* Character : AllCharacters)
	{
		APGCharacterBase* PGCB = Cast<APGCharacterBase>(Character);
		if (PGCB)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s [2nd]"), *PGCB->GetName());
			PGCB->InitSoundManager(SoundManager);
		}
	}
}