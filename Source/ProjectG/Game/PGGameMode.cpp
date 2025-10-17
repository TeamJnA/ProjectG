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
}

/*
* GameInstance에서 Lobby 레벨에서 출발한 플레이어 목록을 읽어 
* 5초 후 실제 도착한 플레이어 목록과 비교 
*/
void APGGameMode::BeginPlay()
{
	Super::BeginPlay();

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	APGGameState* GS = GetGameState<APGGameState>();
	if (!GI || !GS)
	{
		return;
	}

	GS->OnMapGenerationComplete.AddDynamic(this, &APGGameMode::HandleMapGenerationComplete);

	SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
	if (!SoundManager) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
	}

	FTimerHandle TravelCheckTimer;
	GetWorld()->GetTimerManager().SetTimer(
		TravelCheckTimer,
		this,
		&APGGameMode::CheckAllPlayersArrived,
		5.0f,
		false
	);
}

/*
* 레벨 이동에 성공한 플레이어 컨트롤러에서 호출
* 해당 플레이어를 레벨 이동에 성공한 플레이어 목록에 추가
*/
void APGGameMode::PlayerTravelSuccess(APlayerController* Player)
{
	if (Player && Player->PlayerState)
	{
		ArrivedPlayers.Add(Player->PlayerState->GetUniqueId());
	}
}

/*
* 레벨 이동에 성공한 플레이어 목록과 GI에서 읽어온 레벨 이동을 시도한 플레이어 목록 비교
* 실패한 플레이어가 존재하는 경우 해당 플레이어를 세션에서 내보냄
* 모든 플레이어를 확인하고 세션 검색을 막아 게임 중간 참가 방지
* 이후 레벨 생성 작업 시작
*/
void APGGameMode::CheckAllPlayersArrived()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		return;
	}

	const TArray<FUniqueNetIdRepl>& ExpectedPlayers = GI->GetExpectedPlayersForTravel();
	for (const FUniqueNetIdRepl& ExpectedPlayer : ExpectedPlayers)
	{
		if (!ArrivedPlayers.Contains(ExpectedPlayer))
		{
			if (ExpectedPlayer.IsValid())
			{
				GI->KickPlayerFromSession(*ExpectedPlayer.GetUniqueNetId());
			}
		}
	}

	GI->ClearExpectedPlayersForTravel();

	if (!ArrivedPlayers.IsEmpty())
	{
		SpawnLevelGenerator();
		GI->CloseSession();
	}
}

void APGGameMode::HandleMapGenerationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GM::HandleMapGenerationComplete: Recieved OnMapGenerationComplete"));
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnAllPlayers);
}

/*
* 요청한 플레이어 PlayerList의 bIsReadyToReturnLobby를 true로 설정
* 남아있는 모든 플레이어의 bIsReadyToReturnLobby == true 인 경우 -> 세션을 유지하고 로비로 이동 
*/
void APGGameMode::SetPlayerReadyToReturnLobby(APlayerState* PlayerState)
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

			GS->SetCurrentGameState(EGameState::Lobby);
			if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
			{
				GI->SaveGameStateOnTravel(GS->GetCurrentGameState());
				UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Saving GameState to GameInstance before travel."));
			}

			GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGLobbyRoom?listen", true);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Not all players are ready yet"));
		}
	}
}

/*
* 탈출하는 플레이어 상태 설정
* 탈출하는 플레이어를 관전 중인 플레이어가 있다면 Escape 카메라로 관전 시점 변경
*/
void APGGameMode::HandlePlayerEscaping(ACharacter* EscapingPlayer)
{
	if (!EscapingPlayer)
	{
		return;
	}

	APGPlayerState* EscapingPlayerPS = EscapingPlayer->GetPlayerState<APGPlayerState>();
	if (!EscapingPlayerPS)
	{
		return;
	}
	EscapingPlayerPS->SetIsEscaping(true);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APGPlayerController* SpectatorPC = Cast<APGPlayerController>(It->Get());
		if (SpectatorPC)
		{
			if (SpectatorPC->GetCurrentSpectateTargetPlayerState() == EscapingPlayerPS)
			{
				SpectatorPC->ForceSpectateTarget();
			}
		}
	}
}

/*
* 캐릭터 플레이어 스폰
* 각 PC에 Possess
*/
void APGGameMode::SpawnAllPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APGPlayerController* PC = Cast<APGPlayerController>(It->Get());
		if (!PC || !PC->PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: SpawnAllPlayers: PlayerController is nullptr."));
			return;
		}

		const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(920.0f, 0.0f + SpawnOffset, 100.0f));
		APGPlayerCharacter* NewPawn = GetWorld()->SpawnActor<APGPlayerCharacter>(PlayerPawnClass, SpawnTransform);
		if (NewPawn)
		{
			PC->Possess(NewPawn);
		}

		SpawnOffset += 50;

		PC->Client_HideLoadingScreen();
	}

	UE_LOG(LogTemp, Log, TEXT("GameMode: All players spawned. Spawn GlobalLightManager and SoundManager."));
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnGlobalLightManager);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::InitSoundManagerToPlayers);
}

/*
* Level Generator 스폰 
*/
void APGGameMode::SpawnLevelGenerator()
{
	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);	
	APGLevelGenerator* LG = GetWorld()->SpawnActor<APGLevelGenerator>(APGLevelGenerator::StaticClass(), SpawnTransform);
	if (!LG)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: LevelGenerator not Spawned"));
	}
}

/*
* Light Manager 스폰
*/
void APGGameMode::SpawnGlobalLightManager()
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode: Spawn GlobalLightManager"));

	APGGlobalLightManager* LightManager = GetWorld()->SpawnActor<APGGlobalLightManager>(APGGlobalLightManager::StaticClass());
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

APGSoundManager* APGGameMode::GetSoundManager()
{
	return SoundManager;
}