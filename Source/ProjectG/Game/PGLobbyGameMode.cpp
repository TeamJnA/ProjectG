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

	// ���� ����� ���� ���� ä��ϴ�.
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
				// �������� ���� �ƹ�Ÿ �����͸� �����ɴϴ�.
				GI->GetSteamAvatarAsRawData(*UniqueIdRepl.GetUniqueNetId(), Info.AvatarRawData, Info.AvatarWidth, Info.AvatarHeight);
			}

			GS->LobbyPlayerList.Add(Info);
		}
	}

	// ������ UI�� ������Ʈ�� �ʿ��ϴٸ�, OnRep �Լ��� �������� ȣ�����ݴϴ�.
	// OnRep�� ���� Ŭ���̾�Ʈ������ �ڵ� ȣ�������, �������� ���� ȣ���ϴ� ���� �Ϲ����� �����Դϴ�.
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
	
	// GameState�� �̹� AddPlayerState�� ���� ����� ����������,
	// ������ ����� ������ġ�� GameMode������ �� �� �� ������ ��û�մϴ�.
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdateLobbyPlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Failsafe update requested."));
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Player [%s] has login."), *NewPlayer->GetName());

}

void APGLobbyGameMode::Logout(AController* Exiting)
{
	// GameState�� �̹� RemovePlayerState�� ���� ����� ����������,
	// Ÿ�̹� �̽��� �Ϻ��� ȸ���ϰ� �߰����� ������ ���� ���� ƽ�� ������ �� �� �� ��û�մϴ�.
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdateLobbyPlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::Logout: Failsafe (next tick) update requested."));
	}
	UE_LOG(LogTemp, Log, TEXT("LobbyGM::Logout: Player [%s] has logout."), *Exiting->GetName());

	Super::Logout(Exiting);
}
