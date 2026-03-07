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
#include "Character/PGSpectatorPawn.h"

#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

#include "Level/Manager/PGLevelGenerator.h"
#include "Level/Manager/PGGlobalLightManager.h"
#include "UI/Manager/PGHUD.h"
#include "Sound/PGSoundManager.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "Physics/PGChaosCacheManager.h"
#include "PGLobbyGameMode.h"


APGGameMode::APGGameMode()
{
	bStartPlayersAsSpectators = true;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ProjectG/Character/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		PlayerPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APGGameState> PGGameStateClass(TEXT("/Game/ProjectG/Game/BP_PGGameState.BP_PGGameState_C"));
	if (PGGameStateClass.Class != nullptr)
	{
		GameStateClass = PGGameStateClass.Class;
	}

	PlayerStateClass = APGPlayerState::StaticClass();
	PlayerControllerClass = APGPlayerController::StaticClass();
	
	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/ProjectG/UI/Manager/BP_PGHUD"));
	if (HUDBPClass.Class != nullptr)
	{
		HUDClass = HUDBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APGGhostCharacter> GhostPawnBPClass(TEXT("/Game/ProjectG/Enemy/Ghost/Character/BP_GhostCharacter.BP_GhostCharacter_C"));
	if (GhostPawnBPClass.Class != nullptr)
	{
		GhostCharacterClass = GhostPawnBPClass.Class;
	}

	PlayerSpawnTransforms.Add(FTransform(FRotator(0.0f, 20.0f, 0.0f), FVector(2640.0f, 180.0f, -298.6f)));
	PlayerSpawnTransforms.Add(FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(2600.0f, 410.0f, -298.6f)));
	PlayerSpawnTransforms.Add(FTransform(FRotator(0.0f, 50.0f, 0.0f), FVector(2750.0f, -100.0f, -298.6f)));
	PlayerSpawnTransforms.Add(FTransform(FRotator(0.0f, -30.0f, 0.0f), FVector(2690.0f, 710.0f, -298.6f)));

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

	GI->CloseSession();
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

void APGGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (GI)
	{
		const TArray<FUniqueNetIdRepl>& ExpectedPlayers = GI->GetExpectedPlayersForTravel();
		bool bIsExpected = false;
		for (const FUniqueNetIdRepl& ExpectedUser : ExpectedPlayers)
		{
			if (ExpectedUser == UniqueId)
			{
				bIsExpected = true;
				break;
			}
		}

		if (!bIsExpected)
		{
			ErrorMessage = TEXT("Game Started");
			//UE_LOG(LogTemp, Warning, TEXT("[PreLogin] Rejected unexpected player: %s"), *UniqueId.ToString());
		}
	}
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

			// Stop and delete GC
			CleanupGeometryCollections();

			GS->SetCurrentGameState(EGameState::Lobby);
			if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
			{
				GI->SaveGameStateOnTravel(EGameState::Lobby);
				UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Saving GameState to GameInstance before travel."));
			}

			RequestServerTravel();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("GM::SetPlayerReadyToReturnLobby: Not all players are ready yet"));
		}
	}
}

/*
* 캐릭터 플레이어 스폰
* 각 PC에 Possess
*/
void APGGameMode::SpawnAllPlayers()
{
	int32 PlayerIndex = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APGPlayerController* PC = Cast<APGPlayerController>(It->Get());
		if (!PC || !PC->PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: SpawnAllPlayers: PlayerController is nullptr."));
			return;
		}

		APGPlayerCharacter* NewPawn = GetWorld()->SpawnActor<APGPlayerCharacter>(PlayerPawnClass, PlayerSpawnTransforms[PlayerIndex]);
		if (NewPawn)
		{
			// 이곳에서랑 클라이언트에서 두 번 Hidden 처리를 한다.
			// 복제가 잘 안 될 수 있으므로....
			NewPawn->SetActorHiddenInGame(true);

			PC->Possess(NewPawn);
			PC->Client_PlayGameplayBGM();
		}

		PlayerIndex++;


		PC->Client_HideLoadingScreen();
	}

	APGGameState* GS = GetGameState<APGGameState>();
	if (GS)
	{
		GS->Multicast_PlayerEnterLevelSequence(GetWorld()->GetNumPlayerControllers());
	}

	UE_LOG(LogTemp, Log, TEXT("GameMode: All players spawned. Spawn GlobalLightManager and SoundManager."));
	//GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::SpawnGlobalLightManager);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APGGameMode::InitSoundManagerToPlayers);
}

/*
* Level Generator 스폰 
*/
void APGGameMode::SpawnLevelGenerator()
{
	check(PGLevelGenerator);

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);	
	APGLevelGenerator* LG = GetWorld()->SpawnActor<APGLevelGenerator>(PGLevelGenerator, SpawnTransform);
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

void APGGameMode::SpawnGhost(const FTransform& SpawnTransform)
{
	UE_LOG(LogTemp, Log, TEXT("GM::SpawnGhostsForPlayers: Spawning ghosts for all players."));

	APGGameState* GS = GetGameState<APGGameState>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("GM::SpawnGhostsForPlayers: No GameState found."));
		return;
	}

	if (!GhostCharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("APGGameMode::SpawnGhostsForPlayers: GhostCharacterClass is not set in GameMode! Check BP Path."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const float SpawnOffsetRadius = 100.0f;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS)
		{
			FVector RandomOffset = FVector(FMath::RandRange(-SpawnOffsetRadius, SpawnOffsetRadius), FMath::RandRange(-SpawnOffsetRadius, SpawnOffsetRadius), 0.0f);
			FTransform FinalSpawnTransform = SpawnTransform;
			FinalSpawnTransform.AddToTranslation(RandomOffset);

			APGGhostCharacter* NewGhost = GetWorld()->SpawnActor<APGGhostCharacter>(GhostCharacterClass, FinalSpawnTransform, SpawnParams);
			if (NewGhost)
			{
				/*if (SoundManager)
				{
					NewGhost->InitSoundManager(SoundManager);
				}*/

				NewGhost->SetTargetPlayerState(PS);

				UE_LOG(LogTemp, Log, TEXT("APGGameMode: Spawned Ghost (%s) and assigned to Player (%s)"), *NewGhost->GetName(), *PS->GetPlayerName());
			}
		}
	}
}

void APGGameMode::ProcessSoloLeaveRequest(APGPlayerController* RequestingPC, ECleanupActionType ActionType)
{
	if (bIsMassProcessing)
	{
		return;
	}

	if (!RequestingPC || PendingSoloLeavers.Contains(RequestingPC))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("PGGameMode: Solo Leave Requested by %s."), *RequestingPC->GetName());
	PendingSoloLeavers.Add(RequestingPC, ActionType);

	FUniqueNetIdRepl LeaverId;
	if (RequestingPC->PlayerState)
	{
		LeaverId = RequestingPC->PlayerState->GetUniqueId();
	}

	RequestingPC->Client_StopVoiceAndCleanup(ActionType, FUniqueNetIdRepl());
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APGPlayerController* OtherPC = Cast<APGPlayerController>(It->Get());
		if (!OtherPC || OtherPC == RequestingPC || PendingSoloLeavers.Contains(OtherPC))
		{
			continue;
		}

		OtherPC->Client_StopVoiceAndCleanup(ECleanupActionType::None, LeaverId);
	}

	FTimerHandle PersonalTimerHandle;
	FTimerDelegate TimeoutDelegate = FTimerDelegate::CreateUObject(this, &APGGameMode::PerformSoloLeave, RequestingPC);
	GetWorld()->GetTimerManager().SetTimer(PersonalTimerHandle, TimeoutDelegate, 2.0f, false);
	SoloLeaveTimers.Add(RequestingPC, PersonalTimerHandle);
}

void APGGameMode::PerformSoloLeave(APGPlayerController* TargetPC)
{
	if (bIsMassProcessing)
	{
		return;
	}

	if (!TargetPC || !PendingSoloLeavers.Contains(TargetPC))
	{
		UE_LOG(LogTemp, Warning, TEXT("GM: Solo Leave Cancelled."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("GM: Solo Leave Timeout for %s. Forcing Leave."), *TargetPC->GetName());
	ECleanupActionType SoloAction = PendingSoloLeavers[TargetPC];
	if (SoloLeaveTimers.Contains(TargetPC))
	{
		SoloLeaveTimers.Remove(TargetPC);
	}

	TargetPC->Client_ExecuteSoloAction(SoloAction);
}

void APGGameMode::RequestServerTravel()
{
	if (bIsMassProcessing)
	{
		return;
	}
	bIsMassProcessing = true;

	CancelAllPendingSoloLeaves();

	bServerShouldQuit = false;
	PendingActionType = ECleanupActionType::Mass_ServerTravel;

	UE_LOG(LogTemp, Log, TEXT("PGGameMode: Mass Travel Requested. Muting All."));
	BroadcastCleanupCommand();
}

void APGGameMode::RequestSessionDestruction(bool bServerQuit)
{
	if (bIsMassProcessing)
	{
		return;
	}
	bIsMassProcessing = true;

	CancelAllPendingSoloLeaves();

	bServerShouldQuit = bServerQuit;
	PendingActionType = ECleanupActionType::Mass_KickForDestroy;

	UE_LOG(LogTemp, Log, TEXT("PGGameMode: Destroy Session Requested. Muting All."));
	BroadcastCleanupCommand();
}

void APGGameMode::CancelAllPendingSoloLeaves()
{
	if (PendingSoloLeavers.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("GM: Mass Action Triggered. Cancelling %d pending solo leaves."), PendingSoloLeavers.Num());

	for (auto& Elem : SoloLeaveTimers)
	{
		APGPlayerController* PC = Elem.Key;
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

void APGGameMode::BroadcastCleanupCommand()
{
	UE_LOG(LogTemp, Log, TEXT("GM: Broadcasting Mass Cleanup. Execution in 2.0s."));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APGPlayerController* PC = Cast<APGPlayerController>(It->Get()))
		{
			if (PendingSoloLeavers.Contains(PC))
			{
				continue;
			}

			PC->Client_StopVoiceAndCleanup(PendingActionType, FUniqueNetIdRepl());
		}
	}

	FTimerHandle MassActionTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(MassActionTimerHandle, this, &APGGameMode::ExecutePendingAction, 2.0f, false);
}

void APGGameMode::ExecutePendingAction()
{
	if (!bIsMassProcessing)
	{
		return;
	}

	if (PendingActionType == ECleanupActionType::Mass_ServerTravel)
	{
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGLobbyRoom?listen", true);
	}
	else if (PendingActionType == ECleanupActionType::Mass_KickForDestroy)
	{
		APGPlayerController* HostPC = Cast<APGPlayerController>(GetWorld()->GetFirstPlayerController());

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APGPlayerController* PC = Cast<APGPlayerController>(It->Get()))
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

void APGGameMode::CleanupGeometryCollections()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APGChaosCacheManager::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		APGChaosCacheManager* CCM = Cast<APGChaosCacheManager>(Actor);
		if (CCM)
		{
			CCM->Multicast_CleanupGeometyCollection();
		}
	}
}

void APGGameMode::Logout(AController* Exiting)
{
	// 시체(PlayerCharacter) 정리
	if (APGPlayerState* PS = Exiting->GetPlayerState<APGPlayerState>())
	{
		if (AActor* DeadBody = PS->GetPlayerCharacter())
		{
			if (DeadBody != Exiting->GetPawn())
			{
				UE_LOG(LogTemp, Log, TEXT("[GM::Logout] Destroy logout player dead body [%s]"), *PS->GetPlayerName());
				DeadBody->Destroy();
			}
		}

		// 해당 플레이어 담당 Ghost 정리
		for (TActorIterator<APGGhostCharacter> It(GetWorld()); It; ++It)
		{
			if (It->GetTargetPlayerState() == PS)
			{
				UE_LOG(LogTemp, Log, TEXT("[GM::Logout] Destroy logout player ghost [%s]"), *PS->GetPlayerName());
				It->Destroy();
				break;
			}
		}
	}

	Super::Logout(Exiting);

	APGPlayerController* LeavingPC = Cast<APGPlayerController>(Exiting);
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

void APGGameMode::InitSoundManagerToPlayers()
{
	// Set the character's SoundManager pointer to the globally spawned soundmanager instance.
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* PC = it->Get();
		if (!PC) continue;

		APGPlayerCharacter* Character = Cast<APGPlayerCharacter>(PC->GetPawn());
		if (Character)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s"), *Character->GetName());
			Character->InitSoundManager(GetSoundManager());
		}
	}
}

APGSoundManager* APGGameMode::GetSoundManager()
{
	return SoundManager;
}

/*
* 탈출하는 플레이어 상태 설정
* 탈출하는 플레이어를 관전 중인 플레이어가 있다면 Escape 카메라로 관전 시점 변경
*/
void APGGameMode::HandlePlayerEscaping(ACharacter* EscapingPlayer, EExitPointType ExitPointType)
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
	EscapingPlayerPS->SetExitPoint(ExitPointType);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APGPlayerController* SpectatorPC = Cast<APGPlayerController>(It->Get());
		if (!SpectatorPC)
		{
			continue;
		}

		if (SpectatorPC->GetCurrentSpectateTargetPlayerState() == EscapingPlayerPS)
		{
			SpectatorPC->SetSpectateEscapeCamera(ExitPointType);
		}
	}
}

void APGGameMode::RespawnPlayer(AController* DeadPlayerController, const FTransform& SpawnTransform)
{
	APGPlayerController* DeadPC = Cast<APGPlayerController>(DeadPlayerController);
	APGPlayerState* DeadPS = DeadPlayerController->GetPlayerState<APGPlayerState>();
	if (!DeadPC || !DeadPS)
	{
		UE_LOG(LogTemp, Error, TEXT("GM::RespawnPlayer called with a NULL controller or NULL player state"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GM::RespawnPlayer: Respawn player %s"), *DeadPS->GetPlayerName());
	DeadPS->SetSpectating(false);
	DeadPS->SetIsDead(false);
	DeadPS->SetHasFinishedGame(false);

	APawn* OldPawn = DeadPC->GetPawn();
	if (!OldPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("GM::RespawnPlayer: OldPawn was null, proceeding to spawn new character."))
	}
	DeadPC->UnPossess();
	OldPawn->Destroy();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APGPlayerCharacter* NewCharacter = GetWorld()->SpawnActor<APGPlayerCharacter>(PlayerPawnClass, SpawnTransform, SpawnParams);
	if (!NewCharacter || !SoundManager)
	{
		UE_LOG(LogTemp, Error, TEXT("GM::RespawnPlayer: No character or sound manager valid"));
		return;
	}
	DeadPC->Client_OnRevive();
	DeadPC->Possess(NewCharacter);
	NewCharacter->InitSoundManager(SoundManager);
	NewCharacter->OnRevive();

	UpdateSpectatorsTarget(NewCharacter, DeadPS);
}

void APGGameMode::UpdateSpectatorsTarget(const ACharacter* RevivedCharacter, const APlayerState* RevivedPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APGPlayerController* SpectatorPC = Cast<APGPlayerController>(It->Get());
		if (!SpectatorPC || SpectatorPC->PlayerState == RevivedPlayerState)
		{
			continue;
		}

		APGSpectatorPawn* SpectatorPawn = SpectatorPC->GetPawn<APGSpectatorPawn>();
		if (!SpectatorPawn)
		{
			continue;
		}

		if (SpectatorPawn->GetSpectateTargetPlayerState() == RevivedPlayerState)
		{
			UE_LOG(LogTemp, Log, TEXT("GM::UpdateSpectatorsTarget: [%s] Updating target to revived character."), *SpectatorPC->GetName());
			SpectatorPC->SetSpectateNewTarget(RevivedCharacter, RevivedPlayerState);
		}
	}
}
