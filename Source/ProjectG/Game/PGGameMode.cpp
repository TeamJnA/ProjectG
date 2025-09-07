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
#include "Game/PGAdvancedFriendsGameInstance.h"

#include "Level/PGLevelGenerator.h"
#include "Level/PGGlobalLightManager.h"
#include "UI/PGHUD.h"
#include "Sound/PGSoundManager.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"


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
	
	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/ProjectG/UI/BP_PGHUD"));
	if (HUDBPClass.Class != nullptr)
	{
		HUDClass = HUDBPClass.Class;
	}

	DefaultPawnClass = nullptr;

	bUseSeamlessTravel = true;

	// TEST TO REMOVE
	BlindCharacterToSpawnTEST;
	static ConstructorHelpers::FClassFinder<AActor> BlindCharacterRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/BP_BlindCharacter.BP_BlindCharacter_C"));
	if (BlindCharacterRef.Class)
	{
		BlindCharacterToSpawnTEST = BlindCharacterRef.Class;
	}
}

void APGGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance());
		if (GI)
		{
			for (const auto& PlayerState : GI->GetExpectedPlayersForTravel())
			{
				if (PlayerState)
				{
					ExpectedPlayers.Add(PlayerState.Get());
				}
			}

			GI->ClearExpectedPlayersForTravel();
			UE_LOG(LogTemp, Log, TEXT("PGGM::BeginPlay: Expecting %d players to arrive."), ExpectedPlayers.Num());
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

		// Spawn sound manager on the level.
		SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
		if (SoundManager) {
			UE_LOG(LogTemp, Log, TEXT("Sound manager spawn complete."));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
		}

		/*
		* Waiting for all players' travel success information
		*/
		GetWorld()->GetTimerManager().SetTimer(TravelCheckTimer, this, &APGGameMode::CheckAllPlayersArrived, 5.0f, false);
	}

}

void APGGameMode::PlayerTravelSuccess(APlayerController* Player)
{
	if (Player && Player->PlayerState)
	{
		UE_LOG(LogTemp, Log, TEXT("PGGM::PlayerTravelSuccess: Player %s has checked in"), *Player->PlayerState->GetPlayerName());
		ArrivedPlayers.Add(Player->PlayerState);
	}
}

void APGGameMode::CheckAllPlayersArrived()
{
	TArray<TObjectPtr<APlayerState>> FailedPlayers;
	for (const auto& ExpectedPlayer : ExpectedPlayers)
	{
		if (ExpectedPlayer && !ArrivedPlayers.Contains(ExpectedPlayer))
		{
			FailedPlayers.Add(ExpectedPlayer);
		}
	}

	if (!FailedPlayers.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("PGGM::CheckAllPlayersArrived: %d player(s) failed to arrive. Kicking them."), FailedPlayers.Num());

		UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
		if (GI)
		{
			for (const auto& FailedPlayerState : FailedPlayers)
			{
				const FUniqueNetIdRepl& PlayerIdToKick = FailedPlayerState->GetUniqueId();
				if (PlayerIdToKick.IsValid())
				{
					GI->KickPlayerFromSession(*PlayerIdToKick.GetUniqueNetId());
				}
			}
		}
	}

	if (!ArrivedPlayers.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("PGGM::CheckAllPlayersArrived: Starting game with %d arrived players."), ArrivedPlayers.Num());
		SpawnLevelGenerator();
		if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
		{
			GI->CloseSession();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PGGM::CheckAllPlayersArrived: No players arrived. Shutting down session."));
		// TODO: 세션 종료 로직
	}
}

void APGGameMode::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Recieved OnMapGenerationComplete"));

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
}

void APGGameMode::SetPlayerReadyToReturnLobby(APlayerState* PlayerState)
{
	if (!HasAuthority()) 
	{
		return;
	}

	APGGameState* GS = GetGameState<APGGameState>();
	if (GS && PlayerState)
	{
		GS->SetPlayerReadyStateForReturnLobby(PlayerState, true);
		UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Player %s ready state updated"), *PlayerState->GetPlayerName());

		if (GS->IsAllReadyToReturnLobby())
		{
			UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: All players are ready to return lobby"));

			GS->SetCurrentGameState(EGameState::Lobby);
			UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());
			if (GI)
			{
				GI->SaveGameStateOnTravel(GS->GetCurrentGameState());
				UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Saving GameState to GameInstance before travel."));
			}

			UWorld* World = GetWorld();
			if (World)
			{
				UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Server initiated travel to Lobby."));

				World->ServerTravel("/Game/ProjectG/Levels/LV_PGLobbyRoom?listen", true);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to get world"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Not all players are ready yet"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GM::SetPlayerReadyToReturnLobby: Failed to get GS or passed PlayerState"));
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
			//newPawn->InitHUD();
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
	// All player spawned completely.
	// Set basic managers and spawn Enemys.
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: All players spawned. Spawn GlobalLightManager and SoundManager."));
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnGlobalLightManager);

		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::InitSoundManagerToPlayers);

		//SpawnEnemy();
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

void APGGameMode::InitSoundManagerToPlayers()
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
			PGPC->InitSoundManager(GetSoundManager());
		}
	}
}

void APGGameMode::SpawnEnemy()
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode: SpawnEnemy"));		
	FVector SpawnLocation = FVector(920.0f, -50.0f, 100.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	APGBlindCharacter* SpawnedBlindCharacter;
	SpawnedBlindCharacter = GetWorld()->SpawnActor<APGBlindCharacter>(BlindCharacterToSpawnTEST, SpawnLocation, SpawnRotation);
	if (SpawnedBlindCharacter)
	{
		SpawnedBlindCharacter->InitSoundManager(GetSoundManager());
	}
}

APGSoundManager* APGGameMode::GetSoundManager()
{
	return SoundManager;
}