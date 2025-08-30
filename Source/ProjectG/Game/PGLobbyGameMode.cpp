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
		GI->SetExpectedPlayerCount(GameState->PlayerArray.Num());

		UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel check logic [%s]"), *GS->GetName());
		GS->NotifyStartTravel();

	}

	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyGM::StartGame: Start travel"));
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}));
}

void APGLobbyGameMode::UpdateLobbyPlayerList()
{
	APGGameState* GS = GetGameState<APGGameState>();
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GS || !GI) return;

	// 기존 목록을 비우고 새로 채웁니다.
	GS->LobbyPlayerList.Empty();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			FPlayerLobbyInfo Info;
			Info.PlayerName = PGPS->GetPlayerName();
			Info.bIsHost = PGPS->IsHost();

			const FUniqueNetIdRepl& UniqueIdRepl = PGPS->GetUniqueId();
			if (UniqueIdRepl.IsValid())
			{
				// 서버에서 직접 아바타 데이터를 가져옵니다.
				GI->GetSteamAvatarAsRawData(*UniqueIdRepl.GetUniqueNetId(), Info.AvatarRawData, Info.AvatarWidth, Info.AvatarHeight);
			}

			GS->LobbyPlayerList.Add(Info);
		}
	}

	// 서버의 UI도 업데이트가 필요하다면, OnRep 함수를 수동으로 호출해줍니다.
	// OnRep은 원래 클라이언트에서만 자동 호출되지만, 서버에서 수동 호출하는 것은 일반적인 패턴입니다.
	GS->OnLobbyPlayerListUpdated.Broadcast();
}

void APGLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		APGPlayerState* PS = NewPlayer->GetPlayerState<APGPlayerState>();
		if (PS)
		{
			if (GameState && GameState->PlayerArray.Num() == 1)
			{
				PS->SetHost(true);
			}
		}
	}
	
	// GameState가 이미 AddPlayerState를 통해 목록을 갱신했지만,
	// 만약을 대비한 안전장치로 GameMode에서도 한 번 더 갱신을 요청합니다.
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdateLobbyPlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Failsafe update requested."));
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Player [%s] has login."), *NewPlayer->GetName());

}

void APGLobbyGameMode::Logout(AController* Exiting)
{
	// GameState가 이미 RemovePlayerState를 통해 목록을 갱신하지만,
	// 타이밍 이슈를 완벽히 회피하고 추가적인 보장을 위해 다음 틱에 갱신을 한 번 더 요청합니다.
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdateLobbyPlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::Logout: Failsafe (next tick) update requested."));
	}
	UE_LOG(LogTemp, Log, TEXT("LobbyGM::Logout: Player [%s] has logout."), *Exiting->GetName());

	Super::Logout(Exiting);
}
