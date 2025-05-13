// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/PGGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PGPlayerController.h"
#include "Player/PGPlayerState.h"
#include "Game/PGGameState.h"
#include "Level/PGGlobalLightManager.h"

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
}

void APGGameMode::BeginPlay()
{
	Super::BeginPlay();

	// only server
	if (!HasAuthority()) return;

	if (GetWorld())
	{
		APGGameState* gs = GetWorld()->GetGameState<APGGameState>();
		if (gs)
		{
			gs->OnMapGenerationComplete.AddDynamic(this, &APGGameMode::HandleMapGenerationComplete);
			gs->OnSpawnComplete.AddDynamic(this, &APGGameMode::HandleSpawnComplete);
		}
	}
}

void APGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ConnectedPlayerCount++;
	UE_LOG(LogTemp, Warning, TEXT("GameMode: PostLogin. ConnectedPlayerCount = %d"), ConnectedPlayerCount);

	if (bDelegateFailed && ConnectedPlayerCount >= 2)
	{
		HandleMapGenerationComplete();
		HandleSpawnComplete();
	}
}

void APGGameMode::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnMapGenerationComplete"));
	SpawnAllPlayers();	
}

void APGGameMode::HandleSpawnComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnSpawnComplete"));

	if (ConnectedPlayerCount >= 2 && !bManagerSpawned)
	{
		SpawnGlobalLightManager();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Not enough players yet."));
		bDelegateFailed = true;
	}
}

void APGGameMode::SpawnAllPlayers()
{
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* pc = it->Get();
		if (!pc || pc->GetPawn()) continue;

		FVector spawnLoc = FVector(700.0f, 0.0f + SpawnOffset, 100.0f);
		FRotator spawnRot = FRotator::ZeroRotator;

		APawn* newPawn = GetWorld()->SpawnActor<APawn>(PlayerPawnClass, spawnLoc, spawnRot);
		if(newPawn)
		{
			pc->Possess(newPawn);
		}

		SpawnOffset += 50;


		if (APGPlayerController* castedPc = Cast<APGPlayerController>(pc))
		{
			if (castedPc->IsLocalPlayerController() && HasAuthority())
			{
				if (APGGameState* gs = GetGameState<APGGameState>())
				{
					gs->NotifyClientReady(castedPc);
				}
			}
			else
			{
				castedPc->Client_CheckLevelSync();
			}
		}
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
