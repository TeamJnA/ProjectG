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
		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
		if (GS)
		{
			GS->OnMapGenerationComplete.AddDynamic(this, &APGGameMode::HandleMapGenerationComplete);
			GS->OnSpawnComplete.AddDynamic(this, &APGGameMode::HandleSpawnComplete);
			GS->OnClientTravel.AddDynamic(this, &APGGameMode::HandleClientTravel);
		}
	}
}

void APGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ConnectedPlayerCount++;
	UE_LOG(LogTemp, Warning, TEXT("GameMode: HandleStartingNewPlayer. ConnectedPlayerCount = %d"), ConnectedPlayerCount);
}

void APGGameMode::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnMapGenerationComplete"));

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
}

void APGGameMode::HandleSpawnComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnSpawnComplete"));

	if (const APGGameState* GS = GetGameState<APGGameState>())
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: PlayerArray Num: %d"), GS->PlayerArray.Num());
		UE_LOG(LogTemp, Log, TEXT("GameMode: ConnectedPlayerCount: %d"), ConnectedPlayerCount);
		UE_LOG(LogTemp, Log, TEXT("GameMode: bManagerSpawned: %d"), bManagerSpawned);

		if (ConnectedPlayerCount >= GS->PlayerArray.Num() && !bManagerSpawned)
		{
			SpawnGlobalLightManager();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Not enough players yet."));
			bDelegateFailed = true;
		}		
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: GameState is not in"));
	}
}

void APGGameMode::HandleClientTravel()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnClientTravel"));

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

		if (APGPlayerController* PGPC = Cast<APGPlayerController>(PC))
		{
			TWeakObjectPtr<APGPlayerController> weakPC = PGPC;
			FTimerHandle syncHandle;
			GetWorld()->GetTimerManager().SetTimer(syncHandle, FTimerDelegate::CreateLambda([weakPC, this]()
			{
				if (!weakPC.IsValid()) return;
				CheckPlayerReady(weakPC);
			}), 0.2f, false);
		}
	}
	if (!bAllReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("Some Players not spawned yet. Retry..."));
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
	}
}

void APGGameMode::CheckPlayerReady(TWeakObjectPtr<APGPlayerController> WeakPC)
{
	if (!WeakPC.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: CheckPlayerReady: PlayerController is no longer valid"));
		return;
	}

	APGPlayerController* validPC = WeakPC.Get();
	
	if (!validPC) return;

	if (validPC->IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: CheckPlayerReady: Notify server is ready to GameState"));
		if (APGGameState* GS = GetGameState<APGGameState>())
		{
			GS->NotifyClientReady(validPC);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: CheckPlayerReady: Check client is in right level"));
		validPC->Client_CheckLevelSync();
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
