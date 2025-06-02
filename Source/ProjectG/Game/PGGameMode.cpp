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

	/*
	* Waiting for all players' travel success information
	*/
	GetWorld()->GetTimerManager().SetTimer(TravelCheckTimer, this, &APGGameMode::PostTravel, 4.0f, false);
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
}

void APGGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// if game did not start (some players not in game level -> ExpectedPlayerCount > ConnectedPlayerCount)
	if (!bLevelGeneratorSpawned)
	{
		GetWorld()->GetTimerManager().ClearTimer(TravelCheckTimer);

		ExpectedPlayerCount--;
		if (APlayerController* OutPC = Cast<APlayerController>(Exiting))
		{
			if (ClientTravelCompletedPlayersSet.Contains(OutPC))
			{
				ClientTravelCompletedPlayersSet.Remove(OutPC);
				ConnectedPlayerCount--;
			}
		}

		if (ConnectedPlayerCount >= ExpectedPlayerCount)
		{
			UE_LOG(LogTemp, Log, TEXT("GameMode::Logout: ExpectedPlayerCount: %d"), ExpectedPlayerCount);
			UE_LOG(LogTemp, Log, TEXT("GameMode::Logout: ConnectedPlayerCount: %d"), ConnectedPlayerCount);
			UE_LOG(LogTemp, Log, TEXT("GameMode::Logout: bLevelGeneratorSpawned: %d"), bLevelGeneratorSpawned);
			SpawnLevelGenerator();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode::Logout: Not enough players yet."));
			// what to do after failure?
		}
	}
}

void APGGameMode::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnMapGenerationComplete"));

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
}

void APGGameMode::SetIsTravelFailedExist()
{
	bIsTravelFailedExist = true;
}

/*
* if there is travel failed client
* retry server travel
* else
* spawn check can start game and spawn level generator
*/
void APGGameMode::PostTravel()
{
	if (bIsTravelFailedExist)
	{
		UE_LOG(LogTemp, Warning, TEXT("PGGM::PostTravel: Travel failed player detected."));

		UWorld* world = GetWorld();
		if (world)
		{
			if (APGGameState* GS = Cast<APGGameState>(GameState))
			{
				UE_LOG(LogTemp, Warning, TEXT("PGGM::PostTravel: Start travel check logic [%s]"), *GS->GetName());
				GS->NotifyStartTravel();
			}

			world->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
			{
				UE_LOG(LogTemp, Warning, TEXT("PGGM::Server_RequestServerTravel: Start travel"));
				GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
			}));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PGGM::PostTravel: All players travel success."));

		UE_LOG(LogTemp, Log, TEXT("PGGM::PostTravel: ExpectedPlayerCount: %d"), ExpectedPlayerCount);
		UE_LOG(LogTemp, Log, TEXT("PGGM::PostTravel: ConnectedPlayerCount: %d"), ConnectedPlayerCount);
		UE_LOG(LogTemp, Log, TEXT("PGGM::PostTravel: bLevelGeneratorSpawned: %d"), bLevelGeneratorSpawned);

		if (!bLevelGeneratorSpawned)
		{
			if (ConnectedPlayerCount >= ExpectedPlayerCount)
			{
				SpawnLevelGenerator();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PGGM::PostTravel: Not enough players yet."));
				// what to do after failure?
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PGGM::PostTravel: Already level generator spawned."));
		}
	}
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

		FVector spawnLoc = FVector(920.0f, 0.0f + SpawnOffset, 100.0f);
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
