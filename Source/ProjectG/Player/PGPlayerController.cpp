// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"

#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Character/PGSpectatorPawn.h"
#include "Character/PGPlayerCharacter.h"

#include "Level/Exit/PGExitPointBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "UI/Manager/PGHUD.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "Net/VoiceConfig.h"

#include "PGLogChannels.h"
#include "Utils/PGVoiceUtils.h"


APGPlayerController::APGPlayerController()
{
	ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultGameplayMappingContextRef(TEXT("/Game/ProjectG/Character/Input/IMC_Default.IMC_Default"));
	if (DefaultGameplayMappingContextRef.Succeeded())
	{
		DefaultGameplayMappingContext = DefaultGameplayMappingContextRef.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputMappingContext> SpectateMappingContextRef(TEXT("/Game/ProjectG/Character/Input/IMC_Spectate.IMC_Spectate"));
	if (SpectateMappingContextRef.Succeeded())
	{
		SpectateMappingContext = SpectateMappingContextRef.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> ShowPauseMenuActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_ShowPauseMenu.IA_ShowPauseMenu"));
	if (ShowPauseMenuActionObj.Succeeded())
	{
		ShowPauseMenuAction = ShowPauseMenuActionObj.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> SpectateNextActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_SpectateNext.IA_SpectateNext"));
	if (SpectateNextActionObj.Succeeded())
	{
		SpectateNextAction = SpectateNextActionObj.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> SpectatePrevActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_SpectatePrev.IA_SpectatePrev"));
	if (SpectatePrevActionObj.Succeeded())
	{
		SpectatePrevAction = SpectatePrevActionObj.Object;
	}

	//     /Script/Engine.SoundCue'/Game/ProjectG/Sound/BGMSoundCues/Sound_GamePlayBGM.Sound_GamePlayBGM'
	static ConstructorHelpers::FObjectFinder<USoundBase> GameplayBGMAssetRef(TEXT("/Game/ProjectG/Sound/BGMSoundCues/Sound_GamePlayBGM.Sound_GamePlayBGM"));
	if (GameplayBGMAssetRef.Succeeded())
	{
		GameplayBGMSound = GameplayBGMAssetRef.Object;
	}

	bGameStartFirstSpawned = true;
}

void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

	bShowMouseCursor = false;

	UE_LOG(LogTemp, Log, TEXT("CLIENT/SERVER: %s BeginPlay. IsLocalController: %d, Role: %s"),
		*GetName(), IsLocalController(), *UEnum::GetValueAsString(GetLocalRole()));

	InitializeGameplayBGM();
}

void APGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// ESC
		EnhancedInputComponent->BindAction(ShowPauseMenuAction, ETriggerEvent::Started, this, &APGPlayerController::OnShowPauseMenu);
		// up/down
		EnhancedInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &APGPlayerController::OnSpectateNext);
		EnhancedInputComponent->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &APGPlayerController::OnSpectatePrev);
	}
}

// server
void APGPlayerController::OnPossess(APawn* NewPawn)
{
	// Changing mapping context by pawn( Default gameplay or Spectate )
	Super::OnPossess(NewPawn);

	if (IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("APGPlayerController::OnPossess new pawn [%s]"), *NewPawn->GetName());

		ReplaceInputMappingContext(NewPawn);
		InitLocalVoice();

		if (const APGSpectatorPawn* Spectator = Cast<APGSpectatorPawn>(NewPawn))
		{
			if (APGHUD* HUD = GetHUD<APGHUD>())
			{
				HUD->InitSpectatorWidget();
			}
		}
	}
}

// when Client possess pawn, 
void APGPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		ReplaceInputMappingContext(GetPawn());
		InitLocalVoice();

		if (const APGSpectatorPawn* Spectator = GetPawn<APGSpectatorPawn>())
		{
			if (APGHUD* HUD = GetHUD<APGHUD>())
			{
				HUD->InitSpectatorWidget();
			}
		}
	}
}

void APGPlayerController::InitLocalVoice()
{
	StartTalking();

	FTimerHandle InputDeviceTimer;
	GetWorld()->GetTimerManager().SetTimer(InputDeviceTimer, [this]()
	{
		PGVoiceUtils::ApplySavedInputDevice(GetWorld());
	}, 0.5f, false);
}

void APGPlayerController::SetupPlayerForGameplay()
{
	if (APGPlayerCharacter* PGCharacter = GetPawn<APGPlayerCharacter>())
	{
		// Set camera to player character
		SetViewTargetWithBlend(PGCharacter, 0.1f);

		// Set input enable
		PGCharacter->EnableInput(this);

		PGCharacter->InitHUD();
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	bGameStartFirstSpawned = false;
}

void APGPlayerController::Client_DisplayJumpscare_Implementation(UTexture2D* JumpscareTexture)
{
	if (IsLocalController())
	{
		if (APGHUD* HUD = GetHUD<APGHUD>())
		{
			HUD->DisplayJumpscare(JumpscareTexture);
		}
	}
}

void APGPlayerController::ReplaceInputMappingContext(const APawn* PawnType)
{
	if (!PawnType)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// [NOT COMPLETE]
		//  현재 priority는 1. 나중에 controller 전용 esc 매핑 콘텍스트를 추가할 예정이 있을 수 있음
		if (PawnType->IsA(APGPlayerCharacter::StaticClass()))
		{
			if (Subsystem->HasMappingContext(SpectateMappingContext))
			{
				Subsystem->RemoveMappingContext(SpectateMappingContext);
			}
			Subsystem->AddMappingContext(DefaultGameplayMappingContext, 1);
		}
		else
		{
			if (Subsystem->HasMappingContext(DefaultGameplayMappingContext))
			{
				Subsystem->RemoveMappingContext(DefaultGameplayMappingContext);
			}
			Subsystem->AddMappingContext(SpectateMappingContext, 1);
		}
	}
}

void APGPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// On travel first try success
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::PostSeamlessTravel: [%s] travel success"), *GetNameSafe(this));
	Client_PostSeamlessTravel();
}

void APGPlayerController::Client_PostSeamlessTravel_Implementation()
{
	if (!IsLocalController()) 
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Client_PostSeamlessTravel_Implementation: No valid GI"));
		return;
	}
	GI->ShowLoadingScreen();

	Server_ReportTravelSuccess();
}

void APGPlayerController::Server_ReportTravelSuccess_Implementation()
{
	if (APGGameMode* GM = GetWorld()->GetAuthGameMode<APGGameMode>())
	{
		GM->PlayerTravelSuccess(this);
	}
}

void APGPlayerController::Client_ForceReturnToLobby_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("PGPC::Client_ForceReturnToLobby: Received command from host to leave session"));

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
}

void APGPlayerController::Client_ShowLoadingScreen_Implementation()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Client_HidLoadingScreen: No valid GI"));
		return;
	}
	GI->ShowLoadingScreen();
}

void APGPlayerController::Client_HideLoadingScreen_Implementation()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Client_HidLoadingScreen: No valid GI"));
		return;
	}
	GI->HideLoadingScreen();
}

/*
* 로비로 나가기 버튼을 누른 경우 로컬 플레이어의 선택을 ServerRPC로 GameMode에 전달
*/
void APGPlayerController::NotifyReadyToReturnLobby()
{
	if (!IsLocalController())
	{
		return;
	}

	Server_SetReadyToReturnLobby();
}

/*
* 
*/
void APGPlayerController::Server_SetReadyToReturnLobby_Implementation()
{
	APGGameMode* GM = Cast<APGGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !PlayerState)
	{
		return;
	}
	GM->SetPlayerReadyToReturnLobby(PlayerState);
}

/*
* GM에서 호출
* 탈출하려는 플레이어를 관전 중인 경우
* 해당 플레이어가 탈출을 시작하면 Escape 카메라를 찾아 SetSpectateTarget
*/
void APGPlayerController::SetSpectateEscapeCamera(EExitPointType ExitPoint)
{
	if (!HasAuthority())
	{
		return;
	}

	//////// 유효성 검사
	
	// Get Player State
	APGPlayerState* EscapingPlayerState = SpectateTargetList.IsValidIndex(CurrentSpectateIndex)	? Cast<APGPlayerState>(SpectateTargetList[CurrentSpectateIndex])	: nullptr;
	if (!EscapingPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ForceEscapeSpectate: EscapingPlayerState is invalid at index %d"), CurrentSpectateIndex);
		Server_ChangeSpectateTarget(true);
		return;
	}

	// Cast GetPawn to SpectatorPawn
	APGSpectatorPawn* Spectator = GetPawn<APGSpectatorPawn>();
	if (!Spectator)
	{
		UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: Spectator Pawn not found!"));
		return;
	}

	// Cast PGGameState
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: World is null!"));
		return;
	}

	APGGameState* GS = World->GetGameState<APGGameState>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: GameState is null!"));
		return;
	}
	///////// 유효성 검사 끝
	
	// Get Camera and Start Spectate
	AActor* ExitCamera = GS->GetExitCameraByEnum(ExitPoint);

	if (ExitCamera)
	{
		Spectator->SetSpectateTarget(ExitCamera, EscapingPlayerState);
		UE_LOG(LogTemp, Log, TEXT("ForceEscapeSpectate: Success. ExitPoint: %d"), (int32)ExitPoint);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ForceEscapeSpectate: ExitCamera not found for ExitPoint: %d"), (int32)ExitPoint);
		Server_ChangeSpectateTarget(true);
	}
}

/*
* 관전 대상 강제 지정
*/
void APGPlayerController::SetSpectateNewTarget(const AActor* NewTarget, const APlayerState* NewTargetPlayerState)
{
	if (!HasAuthority())
	{
		return;
	}

	APGSpectatorPawn* Spectator = GetPawn<APGSpectatorPawn>();
	if (!Spectator)
	{
		UE_LOG(LogTemp, Warning, TEXT("PGPC::SetSpectateNewTarget: Player is not spectating"));
		return;
	}
	Spectator->SetSpectateTarget(NewTarget, NewTargetPlayerState);
}

/*
* 현재 관전 대상 return
*/
const APlayerState* APGPlayerController::GetCurrentSpectateTargetPlayerState() const
{
	if (SpectateTargetList.IsValidIndex(CurrentSpectateIndex))
	{
		return SpectateTargetList[CurrentSpectateIndex];
	}
	return nullptr;
}

/*
* 플레이어 부활시 뷰포트, 인풋모드 초기화
*/
void APGPlayerController::Client_OnRevive_Implementation()
{
	if (IsLocalController())
	{
		if (APGHUD* HUD = GetHUD<APGHUD>())
		{
			HUD->ClearViewport();
		}

		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

/*
* 플레이어 탈출 -> 플레이어 입력 비활성화 -> 목표 지점으로 이동 -> Escape 카메라로 시점 변환
*/
void APGPlayerController::Client_StartEscapeSequence_Implementation(const EExitPointType ExitPoint, const bool bNeedAutomove, const FVector AutomoveLocation)
{
	APGPlayerCharacter* PlayerCharacter = GetPawn<APGPlayerCharacter>();
	if (!PlayerCharacter)
	{
		return;
	}

	//////// 유효성 검사
	// Cast PGGameState
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: World is null!"));
		return;
	}

	APGGameState* GS = World->GetGameState<APGGameState>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: GameState is null!"));
		return;
	}
	///////// 유효성 검사 끝

	// Get Camera and Start Spectate
	AActor* ExitCamera = GS->GetExitCameraByEnum(ExitPoint);
	if (!ExitCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Escape sequence actors not found. Requesting immediate escape."));
		OnEscapeMovementFinished();
		return;
	}

	PlayerCharacter->ClearCharacterAbility();
	if (bNeedAutomove)
	{
		const FVector Destination = AutomoveLocation;

		PlayerCharacter->DisableInput(this);
		PlayerCharacter->OnAutomatedMovementCompleted.AddUniqueDynamic(this, &APGPlayerController::OnEscapeMovementFinished);
		PlayerCharacter->StartAutomatedMovement(Destination);
	}
	else
	{
		OnEscapeMovementFinished();
	}

	APGExitPointBase* ExitPointActor = Cast<APGExitPointBase>(ExitCamera);

	SetViewTarget(ExitCamera);
	SetControlRotation(ExitPointActor->GetCameraRoation());
}

/*
* 목적지 도착 -> GS 상태 처리, 플레이어 캐릭터 제거
*/
void APGPlayerController::OnEscapeMovementFinished()
{
	APGPlayerCharacter* PlayerCharacter = GetPawn<APGPlayerCharacter>();
	if (!PlayerCharacter)
	{
		return;
	}
	PlayerCharacter->OnAutomatedMovementCompleted.RemoveDynamic(this, &APGPlayerController::OnEscapeMovementFinished);

	Server_RequestFinishEscape();
}

/*
* GS 상태 처리, 플레이어 캐릭터 제거
*/
void APGPlayerController::Server_RequestFinishEscape_Implementation()
{
	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		if (PlayerState)
		{
			GS->NotifyPlayerFinished(PlayerState);
		}
	}

	if (APawn* MyPawn = GetPawn())
	{
		UnPossess();
		MyPawn->Destroy();
	}
}

/*
* 관전 시작 환경 구성
* ServerRPC를 통해 서버에 관전용 pawn, 관전 대상 설정 요청
*/
void APGPlayerController::StartSpectate()
{
	if (!IsLocalController()) 
	{
		return;
	}

	if (APGHUD* HUD = GetHUD<APGHUD>())
	{
		HUD->ClearViewport();
	}

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	EnableInput(this);

	Server_EnterSpectatorMode();

	UE_LOG(LogTemp, Log, TEXT("PC::StartSpectate: Client requested EnterSpectatorMode."));
}

/*
* 관전용 pawn 스폰, 관전 대상 탐색/저장
*/
void APGPlayerController::Server_EnterSpectatorMode_Implementation()
{
	if (Cast<APGSpectatorPawn>(GetPawn()))
	{
		return;
	}

	SpectateTargetList.Empty();
	CurrentSpectateIndex = -1;

	if (APGPlayerState* LocalPS = GetPlayerState<APGPlayerState>())
	{
		LocalPS->SetSpectating(true);
	}
	
	if (const APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (PS && PS != this->PlayerState)
			{
				SpectateTargetList.Add(PS);
			}
		}
	}

	if (SpectateTargetList.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_EnterSpectatorMode: No Spectate Target"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true; 

	APGSpectatorPawn* Spectator = GetWorld()->SpawnActor<APGSpectatorPawn>(APGSpectatorPawn::StaticClass(), SpawnParams);
	if (!Spectator)
	{
		return;
	}
	Possess(Spectator);
	Server_ChangeSpectateTarget(true);
}

/*
* 관전 대상 변경
* bNext == true -> 다음 대상
* bNext == false -> 이전 대상
* 관전 대상이 탈출한 플레이어인 경우 Escape 카메라 SetSpectateTarget
*/
void APGPlayerController::Server_ChangeSpectateTarget_Implementation(bool bNext)
{
	if (SpectateTargetList.IsEmpty())
	{
		return;
	}

	const int32 NewIndex = bNext 
		? (CurrentSpectateIndex + 1) % SpectateTargetList.Num()
		: (CurrentSpectateIndex - 1 + SpectateTargetList.Num()) % SpectateTargetList.Num();
	if (NewIndex == CurrentSpectateIndex)
	{
		return;
	}
	CurrentSpectateIndex = NewIndex;

	APGSpectatorPawn* Spectator = GetPawn<APGSpectatorPawn>();
	if (!Spectator)
	{
		return;
	}

	const APGPlayerState* NewTargetPS = SpectateTargetList.IsValidIndex(CurrentSpectateIndex) ? Cast<APGPlayerState>(SpectateTargetList[CurrentSpectateIndex]) : nullptr;
	if (!NewTargetPS)
	{
		return;
	}

	if (NewTargetPS->IsEscaping())
	{
		//////// 유효성 검사
		// Cast PGGameState
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: World is null!"));
			return;
		}

		APGGameState* GS = World->GetGameState<APGGameState>();
		if (!GS)
		{
			UE_LOG(LogTemp, Error, TEXT("ForceEscapeSpectate: GameState is null!"));
			return;
		}
		///////// 유효성 검사 끝

		EExitPointType TargetExitPoint = NewTargetPS->GetExitPoint();

		AActor* ExitCamera = GS->GetExitCameraByEnum(TargetExitPoint);
		if (ExitCamera)
		{
			Spectator->SetSpectateTarget(ExitCamera, NewTargetPS);
		}
	}
	else
	{
		AActor* TargetPawn = NewTargetPS->GetPlayerCharacter();
		if (TargetPawn)
		{
			UE_LOG(LogTemp, Log, TEXT("PGPC::Server_ChangeSpectateTarget: TargetPawn: [%s]"), *NewTargetPS->GetPlayerName());
			Spectator->SetSpectateTarget(TargetPawn, NewTargetPS);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPC::Server_ChangeSpectateTarget: Spectate target is not valid"));
		}
	}
}

/*
* up -> 관전 대상을 다음 대상으로 변경
*/
void APGPlayerController::OnSpectateNext(const FInputActionValue& Value)
{
	if (!IsLocalController()) 
	{
		return;
	}
	Server_ChangeSpectateTarget(true);
	UE_LOG(LogTemp, Log, TEXT("PC::OnSpectateNext: Client requested next spectate target."));
}

/*
* down -> 관전 대상을 이전 대상으로 변경
*/
void APGPlayerController::OnSpectatePrev(const FInputActionValue& Value)
{
	if (!IsLocalController()) 
	{	
		return;
	}
	Server_ChangeSpectateTarget(false);
	UE_LOG(LogTemp, Log, TEXT("PC::OnSpectatePrev: Client requested previous spectate target."));
}

/*
* HUD에 FinalScoreBoardWidget 생성 요청
*/
void APGPlayerController::InitFinalScoreBoardWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (APGHUD* HUD = Cast<APGHUD>(GetHUD()))
	{
		HUD->InitFinalScoreBoardWidget();
	}
}

void APGPlayerController::OnShowPauseMenu(const FInputActionValue& Value)
{
	if (!IsLocalController()) 
	{
		return;
	}

	if (APGHUD* HUD = Cast<APGHUD>(GetHUD()))
	{
		HUD->InitPauseMenuWidget();
	}
}

void APGPlayerController::InitializeGameplayBGM()
{
	if (!GameplayBGMSound)
	{
		UE_LOG(LogPGController, Warning, TEXT("Cannot find BGM"));
		return;
	}
	GameplayBGMPlayer = UGameplayStatics::CreateSound2D(this, GameplayBGMSound, 1.0f, 1.0f, 0.0f, nullptr, false);
	GameplayBGMPlayer->bIsUISound = true;
}

void APGPlayerController::Client_PlayGameplayBGM_Implementation()
{
	if (GameplayBGMPlayer && GameplayBGMPlayer->GetSound() && !GameplayBGMPlayer->IsPlaying())
	{
		GameplayBGMPlayer->Play();
	}
}

void APGPlayerController::StopGameplayBGM()
{
	if (GameplayBGMPlayer && GameplayBGMPlayer->IsPlaying())
	{
		GameplayBGMPlayer->FadeOut(1.5f, 0.0f);
	}
}

void APGPlayerController::RefreshVoiceChannel()
{
	if (bIsLeavingSession)
	{
		UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] PC is in Leaving process"));
		return;
	}

	APGPlayerState* MyPS = GetPlayerState<APGPlayerState>();
	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(GetWorld());

	FString NetModeStr = (GetNetMode() == NM_Client) ? TEXT("Client") : TEXT("Server");

	if (!MyPS || !GS || !VoiceInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] [%s] RefreshVoiceChannel Early Exit! (MyPS: %d, GS: %d, VoiceInt: %d)"), *NetModeStr, MyPS != nullptr, GS != nullptr, VoiceInterface.IsValid());
		return;
	}

	bool bIsGameEnd = GS->GetCurrentGameState() == EGameState::EndGame;
	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] --- Start refreshing voice channels for %s ---"), *NetModeStr, *MyPS->GetPlayerName());

	for (APlayerState* OtherPS : GS->PlayerArray)
	{
		APGPlayerState* OtherPGPS = Cast<APGPlayerState>(OtherPS);
		if (!OtherPGPS)
		{
			continue;
		}

		if (OtherPGPS->IsMyPlayerState())
		{
			continue;
		}

		FUniqueNetIdRepl OtherId = OtherPS->GetUniqueId();
		if (!OtherId.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] [%s] Target (%s) UniqueId is invalid! Cannot control voice channel."), *NetModeStr, *OtherPGPS->GetPlayerName());
			continue;
		}

		if (Leavers.Contains(OtherId))
		{
			UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] [%s] Target (%s) is leaving! Cannot control voice channel."), *NetModeStr, *OtherPGPS->GetPlayerName());
			continue;
		}

		VoiceInterface->RegisterRemoteTalker(*OtherId);

		/* 상황 (Target, Source, 3D 음성) */
		// FinalScoreBoard (모두, 모두, x)
		if (bIsGameEnd)
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 1: GameEnd] Target (%s) -> UNMUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());
			VoiceInterface->UnmuteRemoteTalker(0, *OtherId, false);
		}
		// 사망/탈출 후 스코어보드 (x, 인게임, x)
		else if (MyPS->IsInScoreBoard())
		{
			if (OtherPGPS->IsInGame())
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 1: Me-Scoreboard / Target-InGame] Target (%s) -> UNMUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());

				VoiceInterface->UnmuteRemoteTalker(0, *OtherId, false);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 2: Me-Scoreboard / Target-Not InGame] Target (%s) -> MUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());

				VoiceInterface->MuteRemoteTalker(0, *OtherId, false);
			}
			//VoiceInterface->MuteRemoteTalker(0, *OtherId, false);
		}
		// 관전 (관전자, 인게임/관전자, x)
		else if (MyPS->IsSpectating())
		{
			if (OtherPGPS->IsSpectating() || OtherPGPS->IsInGame())
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 3: Me-Spectating / Target-InGame or Spectating] Target (%s) -> UNMUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());
				VoiceInterface->UnmuteRemoteTalker(0, *OtherId, false);
			}
			else // 스코어보드 보는 사람은 차단
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 3: Me-Spectating / Target-Scoreboard] Target (%s) -> MUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());
				VoiceInterface->MuteRemoteTalker(0, *OtherId, false);
			}
		}
		// 인게임 (모두, 인게임 플레이어, o)
		else if (MyPS->IsInGame())
		{
			if (OtherPGPS->IsInGame())
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 4: Me-InGame / Target-InGame] Target (%s) -> UNMUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());
				VoiceInterface->UnmuteRemoteTalker(0, *OtherId, false);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] [Rule 4: Me-InGame / Target-Not InGame] Target (%s) -> MUTE"), *NetModeStr, *OtherPGPS->GetPlayerName());
				VoiceInterface->MuteRemoteTalker(0, *OtherId, false);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s]"), *VoiceInterface->GetVoiceDebugState());

	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] --- Voice channel refresh complete ---"), *NetModeStr);
}

void APGPlayerController::Client_StopVoiceAndCleanup_Implementation(ECleanupActionType ActionType, const FUniqueNetIdRepl& TargetNetId)
{
	if (bIsLeavingSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] Ignored Cleanup Command because PC is already leaving."));
		return;
	}

	if (ActionType != ECleanupActionType::None)
	{
		if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
		{
			GI->ShowLoadingScreen();
		}
	}

	PerformCleanup(TargetNetId);
}

void APGPlayerController::PerformCleanup(const FUniqueNetIdRepl& TargetNetId)
{
	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] --- Perform Cleanup ---"));

	IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(GetWorld());
	if (!VoiceInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] PerformCleanup Early Exit! VoiceInterface Invalid"));
		return;
	}

	if (TargetNetId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] Muting specific leaver"));
		Leavers.Add(TargetNetId);
		VoiceInterface->RegisterRemoteTalker(*TargetNetId);
		VoiceInterface->MuteRemoteTalker(0, *TargetNetId, false);
	}
	else
	{
		bIsLeavingSession = true;
		StopTalking();

		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] Muting ALL players (I am leaving)."));
		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
		if (!GS)
		{
			UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] PerformCleanup Early Exit! GS Invalid"));
			return;
		}
		
		for (APlayerState* OtherPS : GS->PlayerArray)
		{
			APGPlayerState* OtherPGPS = Cast<APGPlayerState>(OtherPS);
			if (!OtherPGPS)
			{
				continue;
			}
	
			if (OtherPGPS->IsMyPlayerState())
			{
				continue;
			}

			FUniqueNetIdRepl OtherId = OtherPS->GetUniqueId();
			if (!OtherId.IsValid())
			{
				continue;
			}

			VoiceInterface->RegisterRemoteTalker(*OtherId);
			VoiceInterface->MuteRemoteTalker(0, *OtherId, false);
		}
		VoiceInterface->ClearVoicePackets();
	}

	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s]"), *VoiceInterface->GetVoiceDebugState());
}

void APGPlayerController::Client_ExecuteSoloAction_Implementation(ECleanupActionType ActionType)
{
	PerformSessionEndAction(ActionType);
}

void APGPlayerController::PerformSessionEndAction(ECleanupActionType ActionType)
{
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		if (ActionType == ECleanupActionType::Solo_ReturnToMainMenu || ActionType == ECleanupActionType::Mass_KickForDestroy)
		{
			GI->LeaveSessionAndReturnToMainMenu();
		}
		else if (ActionType == ECleanupActionType::Solo_QuitToDesktop)
		{
			UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
		}
	}
}

void APGPlayerController::Server_RequestSoloLeave_Implementation(ECleanupActionType ActionType)
{
	if (APGGameMode* GM = Cast<APGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ProcessSoloLeaveRequest(this, ActionType);
	}
}

void APGPlayerController::Server_RequestSessionDestruction_Implementation(bool bServerQuit)
{
	if (APGGameMode* GM = Cast<APGGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->RequestSessionDestruction(bServerQuit);
	}
}
