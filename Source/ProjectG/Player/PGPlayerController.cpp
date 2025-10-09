// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"

#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Character/PGSpectatorPawn.h"
#include "Character/PGPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "UI/PGFinalScoreBoardWidget.h"
#include "UI/PGPauseMenuWidget.h"
#include "UI/PGHUD.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

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
}

// when Client possess pawn, 
void APGPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (!IsLocalController())
	{
		return;
	}

	ReplaceInputMappingContext(GetPawn());
}

void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

	bShowMouseCursor = false;

	UE_LOG(LogTemp, Log, TEXT("CLIENT/SERVER: %s BeginPlay. IsLocalController: %d, Role: %s"),
		*GetName(), IsLocalController(), *UEnum::GetValueAsString(GetLocalRole()));
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

void APGPlayerController::OnPossess(APawn* NewPawn)
{
	// Changing mapping context by pawn( Default gameplay or Spectate )
	Super::OnPossess(NewPawn);

	UE_LOG(LogTemp, Log, TEXT("APGPlayerController::OnPossess new pawn [%s]"), *NewPawn->GetName());

	ReplaceInputMappingContext(NewPawn);
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

/*
* 로비로 나가기 버튼을 누른 경우 로컬 플레이어의 선택을 ServerRPC로 GameMode에 전달
*/
void APGPlayerController::NotifyReadyToReturnLobby()
{
	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Error, TEXT("PC::NotifyReturnToLobby: PC is not local"));
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
	if (GM && PlayerState)
	{
		GM->SetPlayerReadyToReturnLobby(PlayerState);
	}
}

/*
* GM에서 호출
* 탈출하려는 플레이어를 관전 중인 경우
* 해당 플레이어가 탈출을 시작하면 Escape 카메라를 찾아 SetSpectateTarget
*/
void APGPlayerController::ForceSpectateTarget()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("EscapeCutsceneCamera"), FoundCameras);
	ACameraActor* TargetCamera = FoundCameras.Num() > 0 ? Cast<ACameraActor>(FoundCameras[0]) : nullptr;

	if (APGSpectatorPawn* Spectator = GetPawn<APGSpectatorPawn>())
	{
		Spectator->SetSpectateTarget(TargetCamera);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PGPC::ForceEscapeSpectate: EscapeCutsceneCamera not found! Switching to next target as a fallback."));
		Server_ChangeSpectateTarget(true);
	}
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
* 플레이어 탈출 -> 플레이어 입력 비활성화 -> 목표 지점으로 이동 -> Escape 카메라로 시점 변환
*/
void APGPlayerController::Client_StartEscapeSequence_Implementation()
{
	APGPlayerCharacter* PlayerCharacter = GetPawn<APGPlayerCharacter>();
	if (!PlayerCharacter)
	{
		return;
	}

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("EscapeCutsceneCamera"), FoundCameras);
	ACameraActor* TargetCamera = FoundCameras.Num() > 0 ? Cast<ACameraActor>(FoundCameras[0]) : nullptr;

	const FVector Destination = FVector(2060.0f, 310.0f, 10.0f);

	if (!TargetCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Escape sequence actors not found. Requesting immediate escape."));
		OnEscapeMovementFinished();
		return;
	}

	PlayerCharacter->DisableInput(this);
	PlayerCharacter->OnAutomatedMovementCompleted.AddUniqueDynamic(this, &APGPlayerController::OnEscapeMovementFinished);
	PlayerCharacter->StartAutomatedMovement(Destination);
	//SetViewTargetWithBlend(TargetCamera, 0.2f);	
	SetViewTarget(TargetCamera);
	SetControlRotation(TargetCamera->GetActorRotation());
}

/*
* 목적지 도착 -> GS 상태 처리, 플레이어 캐릭터 제거
*/
void APGPlayerController::OnEscapeMovementFinished()
{
	APGPlayerCharacter* PlayerCharacter = GetPawn<APGPlayerCharacter>();
	if (PlayerCharacter)
	{
		PlayerCharacter->OnAutomatedMovementCompleted.RemoveDynamic(this, &APGPlayerController::OnEscapeMovementFinished);
	}

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

	if (UGameViewportClient* ViewPort = GetWorld()->GetGameViewport())
	{
		ViewPort->RemoveAllViewportWidgets();
	}

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	EnableInput(this);

	Server_EnterSpectatorMode();

	UE_LOG(LogTemp, Warning, TEXT("PC::StartSpectate: Client requested EnterSpectatorMode."));
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

	//APawn* PrevPawn = GetPawn();
	//if (PrevPawn)
	//{
	//	PrevPawn->DisableInput(this);
	//}

	SpectateTargetList.Empty();
	CurrentSpectateIndex = -1;
	
	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (PS && PS != this->PlayerState)
			{
				SpectateTargetList.Add(PS);
			}
		}
	}

	if (SpectateTargetList.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Spectate Target"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true; 

	APGSpectatorPawn* Spectator = GetWorld()->SpawnActor<APGSpectatorPawn>(APGSpectatorPawn::StaticClass(), SpawnParams);
	if (Spectator)
	{
		Possess(Spectator);
		Server_ChangeSpectateTarget(true);
	}
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

	if (bNext)
	{
		CurrentSpectateIndex = (CurrentSpectateIndex + 1) % SpectateTargetList.Num();
	}
	else
	{
		CurrentSpectateIndex = (CurrentSpectateIndex - 1 + SpectateTargetList.Num()) % SpectateTargetList.Num();
	}

	APGSpectatorPawn* Spectator = GetPawn<APGSpectatorPawn>();
	if (!Spectator)
	{
		return;
	}

	APGPlayerState* NewTargetPS = SpectateTargetList.IsValidIndex(CurrentSpectateIndex) ? Cast<APGPlayerState>(SpectateTargetList[CurrentSpectateIndex]) : nullptr;
	if (!NewTargetPS)
	{
		return;
	}

	if (NewTargetPS->IsEscaping())
	{
		TArray<AActor*> FoundCameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("EscapeCutsceneCamera"), FoundCameras);
		if (FoundCameras.Num() > 0)
		{
			Spectator->SetSpectateTarget(FoundCameras[0]);
		}
	}
	else
	{
		AActor* TargetPawn = NewTargetPS->GetPawn();
		if (TargetPawn)
		{
			UE_LOG(LogTemp, Log, TEXT("PGPC::Server_ChangeSpectateTarget: TargetPawn: [%s]"), *NewTargetPS->GetPlayerName());
			Spectator->SetSpectateTarget(TargetPawn);
		}
		else
		{
			// TODO: 뭔가 버그로 대상이 유효하지 않은 경우
			// 다음 대상으로 넘기기
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
