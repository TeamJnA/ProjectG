// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGM_Test.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/PGCharacterBase.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/GameState.h"
#include "Player/PGPlayerState.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"

APGGM_Test::APGGM_Test()
{
	static ConstructorHelpers::FClassFinder<APGGhostCharacter> GhostPawnBPClass(TEXT("/Game/ProjectG/Enemy/Ghost/Character/BP_GhostCharacter.BP_GhostCharacter_C"));
	if (GhostPawnBPClass.Class != nullptr)
	{
		GhostCharacterClass = GhostPawnBPClass.Class;
	}
}

APGSoundManager* APGGM_Test::GetSoundManager()
{
	return SoundManager;
}

void APGGM_Test::BeginPlay()
{
	Super::BeginPlay();

	// Spawn sound manager on the lever.
	SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
	if (SoundManager) {
		UE_LOG(LogTemp, Log, TEXT("Sound manager spawn complete."));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APGGM_Test::InitWorld, 4.0f, false);
}

void APGGM_Test::InitWorld()
{
	InitSoundManagerToPlayers();
	SpawnGhost();
}

void APGGM_Test::InitSoundManagerToPlayers()
{
	// Set the character's SoundManager pointer to the globally spawned soundmanager instance.
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

	TArray<AActor*> AllGimmicks;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APGTriggerGimmickBase::StaticClass(), AllGimmicks);

	for (AActor* Gmick : AllGimmicks)
	{
		APGTriggerGimmickBase* PGG = Cast<APGTriggerGimmickBase>(Gmick);
		if (PGG)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s [2nd]"), *PGG->GetName());
			PGG->InitSoundManager();
		}
	}
}

void APGGM_Test::SpawnGhost()
{
	UE_LOG(LogTemp, Log, TEXT("GM::SpawnGhostsForPlayers: Spawning ghosts for all players."));

	AGameStateBase* GS = GetGameState<AGameStateBase>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("APGGM_Test::SpawnGhostsForPlayers: No APGGameState found!"));
		return;
	}

	if (!GhostCharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("APGGameMode::SpawnGhostsForPlayers: GhostCharacterClass is not set in GameMode! Check BP Path."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS && PS->GetPawn())
		{
			const FVector PlayerLocation = PS->GetPawn()->GetActorLocation();
			const FVector SpawnLocation = PlayerLocation + FVector(300.0f, 0.0f, 100.0f);
			const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

			APGGhostCharacter* NewGhost = GetWorld()->SpawnActor<APGGhostCharacter>(GhostCharacterClass, SpawnTransform, SpawnParams);
			if (NewGhost)
			{
				NewGhost->SetTargetPlayerState(PS);

				UE_LOG(LogTemp, Log, TEXT("APGGameMode: Spawned Ghost (%s) and assigned to Player (%s)"), *NewGhost->GetName(), *PS->GetPlayerName());
			}
		}
	}
}