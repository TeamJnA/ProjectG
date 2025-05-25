// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PGGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PGPlayerController.h"
#include "Player/PGPlayerState.h"
#include "Character/PGPlayerCharacter.h"
#include "Game/PGGameState.h"
#include "Level/PGGlobalLightManager.h"
#include "Level/PGLevelGenerator.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

APGGameMode::APGGameMode()
{
	bStartPlayersAsSpectators = true;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ProjectG/Character/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		PlayerPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = APGGameState::StaticClass();
	PlayerStateClass = APGPlayerState::StaticClass();
	PlayerControllerClass = APGPlayerController::StaticClass();

	DefaultPawnClass = nullptr;

	bUseSeamlessTravel = true;
}

void APGGameMode::BeginPlay()
{
	Super::BeginPlay();

	// only server
	if (!HasAuthority()) return;

	if (GetWorld())
	{
		UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance());
		if (GI)
		{
			ExpectedPlayerCount = GI->GetExpectedPlayerCount();
			UE_LOG(LogTemp, Warning, TEXT("GameMode::BeginPlay: Set ExpectedPlayerCount = %d"), ExpectedPlayerCount);
		}

		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
		if (GS)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode::BeginPlay: Bind delegate function"));			
			/*
			* ClientTravelComplete -> Spawn LevelGenerator
			* MapGenerationComplete -> Spawn Players
			* SpawnPlayersComplete -> Spawn LightManager
			*/
			GS->OnMapGenerationComplete.AddDynamic(this, &APGGameMode::HandleMapGenerationComplete);
		}
	}
	bGamemodeReady = true;

	if (!bLevelGeneratorSpawned)
	{
		if (ExpectedPlayerCount == 1 && ConnectedPlayerCount == 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode::BeginPlay: Already all player registered, spawn level generator"));
			SpawnLevelGenerator();
		}
	}
}

void APGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// if NewPlayer already in set
	if (ClientTravelCompletedPlayersSet.Contains(NewPlayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode::HandleStartingNewPlayer: [%s] already registered."), *NewPlayer->GetName());
		return;
	}	
	ClientTravelCompletedPlayersSet.Add(NewPlayer);
	ConnectedPlayerCount++;
	UE_LOG(LogTemp, Warning, TEXT("GameMode::HandleStartingNewPlayer: [%s] coneected. ConnectedPlayer = %d"), *NewPlayer->GetName(), ClientTravelCompletedPlayersSet.Num());

	// if GameMode::BiginPlay not called
	if (!bGamemodeReady) return;

	UE_LOG(LogTemp, Log, TEXT("GameMode::HandleStartingNewPlayer: ExpectedPlayerCount: %d"), ExpectedPlayerCount);
	UE_LOG(LogTemp, Log, TEXT("GameMode::HandleStartingNewPlayer: ConnectedPlayerCount: %d"), ConnectedPlayerCount);
	UE_LOG(LogTemp, Log, TEXT("GameMode::HandleStartingNewPlayer: bLevelGeneratorSpawned: %d"), bLevelGeneratorSpawned);

	if (!bLevelGeneratorSpawned)
	{
		if (ConnectedPlayerCount >= ExpectedPlayerCount)
		{
			SpawnLevelGenerator();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode::HandleStartingNewPlayer: Not enough players yet."));
			// what to do after failure?
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode::HandleStartingNewPlayer: Already level generator spawned."));
	}
	
}

void APGGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// if game did not start (some players not in game level -> ExpectedPlayerCount > ConnectedPlayerCount)
	if (!bLevelGeneratorSpawned)
	{
		ExpectedPlayerCount--;
		if (ConnectedPlayerCount >= ExpectedPlayerCount)
		{
			UE_LOG(LogTemp, Log, TEXT("GameMode::LogOut: ExpectedPlayerCount: %d"), ExpectedPlayerCount);
			UE_LOG(LogTemp, Log, TEXT("GameMode::LogOut: ConnectedPlayerCount: %d"), ConnectedPlayerCount);
			UE_LOG(LogTemp, Log, TEXT("GameMode::LogOut: bLevelGeneratorSpawned: %d"), bLevelGeneratorSpawned);
			SpawnLevelGenerator();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode::LogOut: Not enough players yet."));
			// what to do after failure?
		}
	}
}

void APGGameMode::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnMapGenerationComplete"));

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
}

void APGGameMode::SpawnAllPlayers()
{
	bool bAllReady = true;

	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* PC = it->Get();
		if (!PC)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: SpawnAllPlayers: PlayerController is nullptr."));
			bAllReady = false;
			continue;
		}
		if (PC->GetPawn()) continue;
		if (!PC->PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: SpawnAllPlayers: PlayerState is nullptr."));
			bAllReady = false;
			continue;
		}

		FVector spawnLoc = FVector(700.0f, 0.0f + SpawnOffset, 100.0f);
		FRotator spawnRot = FRotator::ZeroRotator;

		// APGPlayerCharacter* newPawn = GetWorld()->SpawnActor<APGPlayerCharacter>(PlayerPawnClass, spawnLoc, spawnRot);
		APGPlayerCharacter* newPawn = GetWorld()->SpawnActorDeferred<APGPlayerCharacter>(PlayerPawnClass, FTransform(spawnRot, spawnLoc), PC);

		if (newPawn)
		{
			newPawn->FinishSpawning(FTransform(spawnRot, spawnLoc));
			PC->Possess(newPawn);
		}

		// need update!!
		SpawnOffset += 50;
	}

	if (!bAllReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("Some Players not spawned yet. Retry..."));
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);

		// need retry count limit
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: All players spawned. Spawn GlobalLightManager."));
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnGlobalLightManager);
	}
}

void APGGameMode::SpawnLevelGenerator()
{
	if (bLevelGeneratorSpawned) return;

	UE_LOG(LogTemp, Warning, TEXT("GameMode: Spawn LevelGenerator"));

	FTransform spawnTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(1.0f, 1.0f, 1.0f));	
	APGLevelGenerator* LG = GetWorld()->SpawnActor<APGLevelGenerator>(APGLevelGenerator::StaticClass(), spawnTransform);
	if (LG)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: LevelGenerator Spawned"));
		bLevelGeneratorSpawned = true;
	}
}

void APGGameMode::SpawnGlobalLightManager()
{
	if (bManagerSpawned) return;

	UE_LOG(LogTemp, Warning, TEXT("GameMode: Spawn GlobalLightManager"));

	FTransform spawnTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(1.0f, 1.0f, 1.0f));

	APGGlobalLightManager* mgr = GetWorld()->SpawnActor<APGGlobalLightManager>(APGGlobalLightManager::StaticClass(), spawnTransform);

	if (mgr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: GlobalLightManager Spawned"));
		bManagerSpawned = true;
	}
}
