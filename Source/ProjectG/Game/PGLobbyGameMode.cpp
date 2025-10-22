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

	// �÷��̾� ���°� ���ŵǾ� ��� Ŭ���̾�Ʈ UI ������Ʈ
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->NotifyPlayerArrayUpdated();
	}
}

/*
* ���� ������ �̵�
* ���� �̵� �� �񱳸� ���� ���� ���� �� �÷��̾� ����� GameInstance�� ����
* ���� ���¸� EGameState::InGame���� ����
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
* Lobby �÷��̾� ĳ���� ���� �� ���� ��Ʈ�ѷ��� Possess
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
