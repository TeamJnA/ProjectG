// Fill out your copyright notice in the Description page of Project Settings.


#include "PGPlayerController.h"

#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

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

void APGPlayerController::StartSpectate()
{
	if (!IsLocalController()) return;

	if (UGameViewportClient* ViewPort = GetWorld()->GetGameViewport())
	{
		ViewPort->RemoveAllViewportWidgets();
	}

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	Server_EnterSpectatorMode();

	UE_LOG(LogTemp, Warning, TEXT("PC::StartSpectate: Client requested EnterSpectatorMode."));
}

void APGPlayerController::Client_ForceReturnToLobby_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("PGPC::Client_ForceReturnToLobby: Received command from host to leave session"));

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->LeaveSessionAndReturnToLobby();
	}
}

void APGPlayerController::NotifyReadyToReturnLobby()
{
	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Error, TEXT("PC::NotifyReturnToLobby: PC is not local"));
		return;
	}

	Server_SetReadyToReturnLobby();
}

void APGPlayerController::Server_SetReadyToReturnLobby_Implementation()
{
	APGGameMode* GM = Cast<APGGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && PlayerState)
	{
		GM->SetPlayerReadyToReturnLobby(PlayerState);
	}
}

void APGPlayerController::Server_EnterSpectatorMode_Implementation()
{
	// 이미 관전 모드에 진입해 있다면 (APGSpectatorPawn을 빙의하고 있다면) 차단
	if (Cast<APGSpectatorPawn>(GetPawn()))
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_EnterSpectatorMode_Implementation: Already in spectator mode. Ignoring request."));
		return;
	}

	APawn* PrevPawn = GetPawn();
	if (PrevPawn)
	{
		// 기존 Pawn의 입력 비활성화
		PrevPawn->DisableInput(this);
	}

	//
	// ---------- Spawn SpectatorPawn -----------------
	//
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true; // 스폰 실패 시 크래시 방지

	// 기존 Pawn의 위치에서 SpectatorPawn 스폰
	FVector SpawnLoc = PrevPawn ? PrevPawn->GetActorLocation() : FVector::ZeroVector;
	FRotator SpawnRot = PrevPawn ? PrevPawn->GetActorRotation() : FRotator::ZeroRotator;

	// 스폰 전에 기존 SpectatorPawn이 있다면 파괴
	if (ControlledSpectator)
	{
		ControlledSpectator->Destroy();
		ControlledSpectator = nullptr;
	}

	ControlledSpectator = GetWorld()->SpawnActor<APGSpectatorPawn>(APGSpectatorPawn::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);

	APGPlayerCharacter* PrevPlayerCharacter = Cast<APGPlayerCharacter>(PrevPawn);
	if (!PrevPlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find PrevPGCharacter in APGPlayerController::Server_EnterSpectatorMode"));
		return;
	}

	// ControlledSpectataor에 관전 대상 캐릭터들 추가.
	// 관전할 다른 대상이 없다면 (솔로플레이인 경우) 관전 모드 진입 방지
	if (!ControlledSpectator->InitCachedAllPlayableCharacters(PrevPlayerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Cannot enter spectator mode. No other players to spectate."));
		return;
	}

	if (ControlledSpectator)
	{
		Possess(ControlledSpectator);

		ControlledSpectator->Server_SetSpectateTarget(true);

		UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode: Spectator mode entered and possessed APGSpectatorPawn for %s."), *GetNameSafe(this));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn APGSpectatorPawn."));
	}
}

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
