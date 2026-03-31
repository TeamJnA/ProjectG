// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGM_Test.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/PGCharacterBase.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"

#include "Level/Misc/PGDoor1.h"

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

void APGGM_Test::SpawnDoorTEST(FVector InLocation, FRotator InRotator)
{
	const FVector SpawnLocation = InLocation;
	const FRotator SpawnRotation = InRotator;
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true;

	check(PGDoor);
	APGDoor1* DefaultDoor = PGDoor->GetDefaultObject<APGDoor1>();
	if (DefaultDoor)
	{
		DefaultDoor->SpawnDoor(GetWorld(), PGDoor, SpawnTransform, SpawnParams, false);
	}
}

void APGGM_Test::BreakDoorTEST(AActor* InteractInvestigator)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 1. 해당 클래스의 모든 액터를 담을 배열 생성
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, APGDoor1::StaticClass(), FoundActors);

	// 2. 배열을 순회하며 함수 실행
	for (AActor* Actor : FoundActors)
	{
		APGDoor1* Door = Cast<APGDoor1>(Actor);
		if (Door)
		{
			// 3. APGDoor1 클래스 내부의 A 함수 실행
			Door->BreakDoorByEnemy(InteractInvestigator);

			// 만약 레벨에 문이 여러 개인데 하나만 실행하고 싶다면 여기서 break;
			// break; 
		}
	}
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
	// SpawnGhost();
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