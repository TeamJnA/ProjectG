// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGLobbyGameMode.h"

#include "Kismet/GameplayStatics.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerState.h"
#include "Character/PGPlayerCharacter.h"

#include "Sound/PGSoundManager.h"

APGLobbyGameMode::APGLobbyGameMode()
{
	PlayerStateClass = APGPlayerState::StaticClass();
	GameStateClass = APGGameState::StaticClass();

	bUseSeamlessTravel = true;
}

void APGLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->OpenSession();
	}

	SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
	if (!SoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
	}
}

void APGLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	UE_LOG(LogTemp, Log, TEXT("LobbyGM::HandleStartingNewPlayer_Implementation:"));

	if (NewPlayer && NewPlayer->IsLocalController())
	{
		if (APGPlayerState* NewPlayerState = NewPlayer->GetPlayerState<APGPlayerState>())
		{
			NewPlayerState->SetHost(true);
		}
	}

	// 플레이어 상태가 갱신되어 모든 클라이언트 UI 업데이트
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->NotifyPlayerArrayUpdated();
	}
}

/*
* 메인 레벨로 이동
* 레벨 이동 후 비교를 위해 현재 세션 내 플레이어 목록을 GameInstance에 저장
* 게임 상태를 EGameState::InGame으로 설정
*/
void APGLobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	APGGameState* GS = GetGameState<APGGameState>();
	if (!GS || !GS)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APGLobbyPlayerController* PC = Cast<APGLobbyPlayerController>(It->Get());
		if (!PC)
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: No valid pc"));
			continue;
		}
		PC->Client_ShowLoadingScreen();
	}

	GI->SaveGameStateOnTravel(EGameState::InGame);
	GI->SetExpectedPlayersForTravel(GS->PlayerArray);

	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}));
}

/*
* Lobby 플레이어 캐릭터 스폰 및 받은 컨트롤러에 Possess
*/
void APGLobbyGameMode::SpawnAndPossessPlayer(APlayerController* NewPlayer)
{
	UWorld* World = GetWorld();
	if (!World || !NewPlayer || !PlayerPawnClass)
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0.0f, -500.0f, 300.0f));
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APGPlayerCharacter* LobbyCharacter = World->SpawnActor<APGPlayerCharacter>(PlayerPawnClass, SpawnTransform, SpawnParams);
	if (LobbyCharacter)
	{
		NewPlayer->Possess(LobbyCharacter);
		LobbyCharacter->InitSoundManager(SoundManager);
	}
}

APGSoundManager* APGLobbyGameMode::GetSoundManager()
{
	return SoundManager;
}
