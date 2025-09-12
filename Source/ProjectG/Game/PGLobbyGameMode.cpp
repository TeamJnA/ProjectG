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

	APawn* LobbyCharacter = World->SpawnActor<APawn>(PlayerPawnClass, SpawnTransform, SpawnParams);
	if (LobbyCharacter)
	{
		NewPlayer->Possess(LobbyCharacter);
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
		
	// PostLogin���� PlayerList ������Ʈ
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdatePlayerList();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Failsafe update requested."));
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyGM::PostLogin: Player [%s] has login."), *NewPlayer->GetName());

}

void APGLobbyGameMode::Logout(AController* Exiting)
{
	// Logout ��Ȳ���� PlayerList ������Ʈ
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
			if (NewPlayer->IsLocalController())
			{
				NewPlayerState->SetHost(true);
			}
			else
			{
				NewPlayerState->SetHost(false);
			}
		}
	}

	// �÷��̾� ���°� ���ŵǾ� ��� Ŭ���̾�Ʈ UI ������Ʈ
	if (APGGameState* GS = GetGameState<APGGameState>())
	{
		GS->UpdatePlayerList();
	}
}
