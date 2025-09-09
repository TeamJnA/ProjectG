// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGLobbyGameMode.h"

#include "Kismet/GameplayStatics.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerState.h"

APGLobbyGameMode::APGLobbyGameMode()
{
	PlayerStateClass = APGPlayerState::StaticClass();
	GameStateClass = APGGameState::StaticClass();

	bUseSeamlessTravel = true;
}

void APGLobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
	APGGameState* GS = Cast<APGGameState>(GameState);
	if (!GS || !GS)
	{
		return;
	}

	GS->SetCurrentGameState(EGameState::InGame);
	GI->SaveGameStateOnTravel(GS->GetCurrentGameState());
	GI->SetExpectedPlayersForTravel(GS->PlayerArray);

	UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel check logic [%s]"), *GS->GetName());

	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel"));
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}));
}

void APGLobbyGameMode::SpawnAndPossessPlayer(APlayerController* NewPlayer)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (!NewPlayer || !PlayerPawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyGM::SpawnAndPossessPlayer: Invalid NewPlayer or DefaultPawnClass"));
		return;
	}

	FVector SpawnLoc = FVector(0.0f, -500.0f, 300.0f);
	FRotator SpawnRot = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APawn* LobbyCharacter = World->SpawnActor<APawn>(PlayerPawnClass, SpawnLoc, SpawnRot, SpawnParams);
	if (LobbyCharacter)
	{
		NewPlayer->Possess(LobbyCharacter);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LobbyGM::SpawnAndPossessPlayer: Failed to spawn lobby character."));
	}
}

void APGLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->OpenSession();
	}
}

void APGLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
		
	// PostLogin에서 PlayerList 업데이트
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdatePlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Failsafe update requested."));
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Player [%s] has login."), *NewPlayer->GetName());

}

void APGLobbyGameMode::Logout(AController* Exiting)
{
	// Logout 상황에서 PlayerList 업데이트
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdatePlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::Logout: Failsafe (next tick) update requested."));
	}
	UE_LOG(LogTemp, Log, TEXT("LobbyGM::Logout: Player [%s] has logout."), *Exiting->GetName());

	Super::Logout(Exiting);
}

void APGLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer)
	{
		if (APGPlayerState* NewPlayerState = NewPlayer->GetPlayerState<APGPlayerState>())
		{
			// Listen Server 환경이라 호스트만 true
			if (NewPlayer->IsLocalController())
			{
				NewPlayerState->SetHost(true);
				UE_LOG(LogTemp, Log, TEXT("LobbyGM::HandleStartingNewPlayer: '%s' is the host (Local Controller)."), *NewPlayerState->GetPlayerName());
			}
			else
			{
				NewPlayerState->SetHost(false);
			}
		}
	}

	// 플레이어 상태가 갱신되어 모든 클라이언트 UI 업데이트
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdatePlayerList();
	}
}
