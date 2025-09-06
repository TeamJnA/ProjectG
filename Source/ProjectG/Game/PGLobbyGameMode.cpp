// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGLobbyGameMode.h"

#include "Kismet/GameplayStatics.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerState.h"

#include "Character/PGPlayerCharacter.h"


APGLobbyGameMode::APGLobbyGameMode()
{
	PlayerStateClass = APGPlayerState::StaticClass();
	GameStateClass = APGGameState::StaticClass();

	bUseSeamlessTravel = true;
}

void APGLobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();
	ensure(World);

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
	ensureMsgf(GI, TEXT("GI is not valid"));
	APGGameState* GS = Cast<APGGameState>(GameState);
	ensureMsgf(GS, TEXT("GS is not valid"));

	if (GI && GS)
	{
		GS->SetCurrentGameState(EGameState::InGame);
		GI->SaveGameStateOnTravel(GS->GetCurrentGameState());
		GI->SetExpectedPlayersForTravel(GS->PlayerArray);

		UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel check logic [%s]"), *GS->GetName());
	}

	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel"));
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}));
}

void APGLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//if (NewPlayer)
	//{
	//	APGPlayerState* PS = NewPlayer->GetPlayerState<APGPlayerState>();
	//	if (PS)
	//	{
	//		if (GameState && GameState->PlayerArray.Num() == 1)
	//		{
	//			PS->SetHost(true);
	//		}
	//	}
	//}
		
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
