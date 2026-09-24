// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameMode_Mansion.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Player/PGPlayerState.h"
#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Physics/PGChaosCacheManager.h"


APGGameMode_Mansion::APGGameMode_Mansion()
{
	static ConstructorHelpers::FClassFinder<APGGhostCharacter> GhostPawnBPClass(TEXT("/Game/ProjectG/Enemy/Ghost/Character/BP_GhostCharacter.BP_GhostCharacter_C"));
	if (GhostPawnBPClass.Class != nullptr)
	{
		GhostCharacterClass = GhostPawnBPClass.Class;
	}

	ReturnTravelURL = TEXT("/Game/ProjectG/Levels/LV_PGLobbyRoom?listen");
}

void APGGameMode_Mansion::SpawnGhost(const FTransform& SpawnTransform)
{
	UE_LOG(LogTemp, Log, TEXT("GM::SpawnGhostsForPlayers: Spawning ghosts for all players."));

	APGGameState* GS = GetGameState<APGGameState>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("GM::SpawnGhostsForPlayers: No GameState found."));
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

	const float SpawnOffsetRadius = 100.0f;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS)
		{
			FVector RandomOffset = FVector(FMath::RandRange(-SpawnOffsetRadius, SpawnOffsetRadius), FMath::RandRange(-SpawnOffsetRadius, SpawnOffsetRadius), 0.0f);
			FTransform FinalSpawnTransform = SpawnTransform;
			FinalSpawnTransform.AddToTranslation(RandomOffset);

			APGGhostCharacter* NewGhost = GetWorld()->SpawnActor<APGGhostCharacter>(GhostCharacterClass, FinalSpawnTransform, SpawnParams);
			if (NewGhost)
			{
				/*if (SoundManager)
				{
					NewGhost->InitSoundManager(SoundManager);
				}*/

				NewGhost->SetTargetPlayerState(PS);

				NewGhost->InitSoundManager(GetSoundManager());

				UE_LOG(LogTemp, Log, TEXT("APGGameMode: Spawned Ghost (%s) and assigned to Player (%s)"), *NewGhost->GetName(), *PS->GetPlayerName());
			}
		}
	}
}

void APGGameMode_Mansion::SetPlayerReadyToReturnLobby(APlayerState* PlayerState)
{
	if (APGPlayerState* PGPS = Cast<APGPlayerState>(PlayerState))
	{
		UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Player %s ready state updated"), *PlayerState->GetPlayerName());
		PGPS->SetReadyToReturnLobby(true);
	}

	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		if (GS->IsAllReadyToReturnLobby())
		{
			UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: All players are ready to return lobby"));

			// Stop and delete GC
			CleanupGeometryCollections();

			GS->SetCurrentGameState(EGameState::Lobby);
			if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
			{
				GI->SaveGameStateOnTravel(EGameState::Lobby);
				UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Saving GameState to GameInstance before travel."));
			}

			RequestServerTravel();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Not all players are ready yet"));
		}
	}
}

void APGGameMode_Mansion::Logout(AController* Exiting)
{
	if (APGPlayerState* PS = Exiting->GetPlayerState<APGPlayerState>())
	{
		// 담당 Ghost 정리
		for (TActorIterator<APGGhostCharacter> It(GetWorld()); It; ++It)
		{
			if (It->GetTargetPlayerState() == PS)
			{
				UE_LOG(LogTemp, Log, TEXT("[GM::Logout] Destroy logout player ghost [%s]"), *PS->GetPlayerName());
				It->Destroy();
				break;
			}
		}
	}

	Super::Logout(Exiting);
}

void APGGameMode_Mansion::CleanupGeometryCollections()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APGChaosCacheManager::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		APGChaosCacheManager* CCM = Cast<APGChaosCacheManager>(Actor);
		if (CCM)
		{
			CCM->Multicast_CleanupGeometyCollection();
		}
	}
}