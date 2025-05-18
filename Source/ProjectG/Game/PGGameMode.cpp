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

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (bDelegateFailed && ConnectedPlayerCount >= GS->PlayerArray.Num())
	{
		HandleMapGenerationComplete();
		HandleSpawnComplete();
	}
}

/*
void APGGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	if (!OldController) return;

	APlayerState* SavedPlayerState = OldController->PlayerState;

	if (OldController->GetPawn())
	{
		OldController->GetPawn()->Destroy();
	}

	OldController->Destroy();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APGPlayerController* NewPC = GetWorld()->SpawnActor<APGPlayerController>(PlayerControllerClass, Params);

	if (NewPC)
	{
		NewPC->PlayerState = SavedPlayerState;
		OldController = NewPC; 
	}

	Super::HandleSeamlessTravelPlayer(C);

	if (APGPlayerController* PC = Cast<APGPlayerController>(C))
	{
		UE_LOG(LogTemp, Log, TEXT("SeamlessTravel: PlayerController retained"));
	}
}
*/

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
}

void APGGameMode::SpawnAllPlayers()
{
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		bool bAllReady = true;

		APlayerController* pc = it->Get();
		if (!pc || pc->GetPawn()) continue;
		if (!pc->PlayerState)
		{
			bAllReady = false;
			continue;
		}

		FVector spawnLoc = FVector(700.0f, 0.0f + SpawnOffset, 100.0f);
		FRotator spawnRot = FRotator::ZeroRotator;

		// APGPlayerCharacter* newPawn = GetWorld()->SpawnActor<APGPlayerCharacter>(PlayerPawnClass, spawnLoc, spawnRot);
		APGPlayerCharacter* newPawn = GetWorld()->SpawnActorDeferred<APGPlayerCharacter>(PlayerPawnClass, FTransform(spawnRot, spawnLoc), pc);

		if (newPawn)
		{
			newPawn->FinishSpawning(FTransform(spawnRot, spawnLoc));
			pc->Possess(newPawn);

			newPawn->InitAbilitySystemComponent();
			newPawn->GiveDefaultAbilities();
			newPawn->InitDefaultAttributes();
			newPawn->GiveAndActivatePassiveEffects();

		}

		SpawnOffset += 50;


		if (APGPlayerController* castedPc = Cast<APGPlayerController>(pc))
		{
			FTimerHandle syncHandle;
			GetWorld()->GetTimerManager().SetTimer(syncHandle, FTimerDelegate::CreateLambda([=, this]()
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
				}), 0.2f, false);
		}

		//if (APGPlayerController* castedPc = Cast<APGPlayerController>(pc))
		//{

		//	if (castedPc->IsLocalPlayerController() && HasAuthority())
		//	{
		//		if (APGGameState* gs = GetGameState<APGGameState>())
		//		{
		//			gs->NotifyClientReady(castedPc);
		//		}
		//	}
		//	else
		//	{
		//		castedPc->Client_CheckLevelSync();
		//	}
		//}

		if (!bAllReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("Some PlayerStates not ready yet. Retry"));
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
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
