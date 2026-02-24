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

void APGLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APGLobbyPlayerController* LeavingPC = Cast<APGLobbyPlayerController>(Exiting);
	if (!LeavingPC)
	{
		return;
	}

	if (SoloLeaveTimers.Contains(LeavingPC))
	{
		GetWorld()->GetTimerManager().ClearTimer(SoloLeaveTimers[LeavingPC]);
		SoloLeaveTimers.Remove(LeavingPC);
	}

	if (PendingSoloLeavers.Contains(LeavingPC))
	{
		PendingSoloLeavers.Remove(LeavingPC);
	}
}

void APGLobbyGameMode::ProcessSoloLeaveRequest(APGLobbyPlayerController* RequestingPC, ECleanupActionType ActionType)
{
	if (bIsMassProcessing)
	{
		return;
	}

	if (!RequestingPC || PendingSoloLeavers.Contains(RequestingPC))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LobbyGM: Solo Leave Requested by %s."), *RequestingPC->GetName());
	PendingSoloLeavers.Add(RequestingPC, ActionType);

	FUniqueNetIdRepl LeaverId;
	if (RequestingPC->PlayerState)
	{
		LeaverId = RequestingPC->PlayerState->GetUniqueId();
	}

	RequestingPC->Client_StopVoiceAndCleanup(ActionType, FUniqueNetIdRepl());
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APGLobbyPlayerController* OtherPC = Cast<APGLobbyPlayerController>(It->Get());
		if (!OtherPC || OtherPC == RequestingPC || PendingSoloLeavers.Contains(OtherPC))
		{
			continue;
		}

		OtherPC->Client_StopVoiceAndCleanup(ECleanupActionType::None, LeaverId);
	}

	FTimerHandle PersonalTimerHandle;
	FTimerDelegate TimeoutDelegate = FTimerDelegate::CreateUObject(this, &APGLobbyGameMode::PerformSoloLeave, RequestingPC);
	GetWorld()->GetTimerManager().SetTimer(PersonalTimerHandle, TimeoutDelegate, 2.0f, false);
	SoloLeaveTimers.Add(RequestingPC, PersonalTimerHandle);
}

void APGLobbyGameMode::PerformSoloLeave(APGLobbyPlayerController* TargetPC)
{
	if (bIsMassProcessing)
	{
		return;
	}

	if (!TargetPC || !PendingSoloLeavers.Contains(TargetPC))
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Solo Leave Cancelled."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Solo Leave Timeout for %s. Forcing Leave."), *TargetPC->GetName());
	ECleanupActionType SoloAction = PendingSoloLeavers[TargetPC];
	if (SoloLeaveTimers.Contains(TargetPC))
	{
		SoloLeaveTimers.Remove(TargetPC);
	}

	TargetPC->Client_ExecuteSoloAction(SoloAction);
}

/*
* 메인 레벨로 이동
* 레벨 이동 후 비교를 위해 현재 세션 내 플레이어 목록을 GameInstance에 저장
* 게임 상태를 EGameState::InGame으로 설정
*/
void APGLobbyGameMode::StartGame()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	APGGameState* GS = GetGameState<APGGameState>();
	if (!GS || !GS)
	{
		return;
	}

	GI->SaveGameStateOnTravel(EGameState::InGame);
	GI->SetExpectedPlayersForTravel(GS->PlayerArray);

	UE_LOG(LogTemp, Log, TEXT("LobbyGM: Start Game (Mass Travel). Muting All."));

	RequestServerTravel();
}

void APGLobbyGameMode::RequestServerTravel()
{
	if (bIsMassProcessing)
	{
		return;
	}
	bIsMassProcessing = true;

	CancelAllPendingSoloLeaves();

	bServerShouldQuit = false;
	PendingActionType = ECleanupActionType::Mass_ServerTravel;

	UE_LOG(LogTemp, Log, TEXT("LobbyGM: Mass Travel Requested. Muting All."));
	BroadcastCleanupCommand();
}

void APGLobbyGameMode::RequestSessionDestruction(bool bServerQuit)
{
	if (bIsMassProcessing)
	{
		return;
	}
	bIsMassProcessing = true;

	CancelAllPendingSoloLeaves();

	bServerShouldQuit = bServerQuit;
	PendingActionType = ECleanupActionType::Mass_KickForDestroy;

	UE_LOG(LogTemp, Log, TEXT("LobbyGM: Destroy Session Requested. Muting All."));
	BroadcastCleanupCommand();
}

void APGLobbyGameMode::CancelAllPendingSoloLeaves()
{
	if (PendingSoloLeavers.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Mass Action Triggered. Cancelling %d pending solo leaves."), PendingSoloLeavers.Num());

	for (auto& Elem : SoloLeaveTimers)
	{
		APGLobbyPlayerController* PC = Elem.Key;
		FTimerHandle& Timer = Elem.Value;

		if (Timer.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(Timer);
		}

		if (PC && PendingSoloLeavers.Contains(PC))
		{
			PendingSoloLeavers.Remove(PC);
		}
	}
	SoloLeaveTimers.Empty();
}

void APGLobbyGameMode::BroadcastCleanupCommand()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyGM: Broadcasting Mass Cleanup. Execution in 2.0s."));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APGLobbyPlayerController* PC = Cast<APGLobbyPlayerController>(It->Get()))
		{
			if (PendingSoloLeavers.Contains(PC))
			{
				continue;
			}

			PC->Client_StopVoiceAndCleanup(PendingActionType, FUniqueNetIdRepl());
		}
	}

	FTimerHandle MassActionTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(MassActionTimerHandle, this, &APGLobbyGameMode::ExecutePendingAction, 2.0f, false);
}

void APGLobbyGameMode::ExecutePendingAction()
{
	if (!bIsMassProcessing)
	{
		return;
	}

	if (PendingActionType == ECleanupActionType::Mass_ServerTravel)
	{
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}
	else if (PendingActionType == ECleanupActionType::Mass_KickForDestroy)
	{
		APGLobbyPlayerController* HostPC = Cast<APGLobbyPlayerController>(GetWorld()->GetFirstPlayerController());

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APGLobbyPlayerController* PC = Cast<APGLobbyPlayerController>(It->Get()))
			{
				if (PC == HostPC)
				{
					continue;
				}

				PC->Client_ExecuteSoloAction(ECleanupActionType::Mass_KickForDestroy);
			}
		}

		if (HostPC)
		{
			if (bServerShouldQuit)
			{
				HostPC->Client_ExecuteSoloAction(ECleanupActionType::Solo_QuitToDesktop);
			}
			else
			{
				HostPC->Client_ExecuteSoloAction(ECleanupActionType::Solo_ReturnToMainMenu);
			}
		}
	}
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
		// TODO: 서버쪽에서 사운드매니저가 안달리는듯?
		//LobbyCharacter->InitSoundManager(SoundManager);

		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this, LobbyCharacter]()
		{
			LobbyCharacter->InitSoundManager(SoundManager);
		}));
	}
}

APGSoundManager* APGLobbyGameMode::GetSoundManager()
{
	return SoundManager;
}
