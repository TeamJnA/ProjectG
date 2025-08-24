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

	ConstructorHelpers::FObjectFinder<UInputAction> OrbitYawActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_OrbitYaw.IA_OrbitYaw"));
	if (OrbitYawActionObj.Succeeded())
	{
		OrbitYawAction = OrbitYawActionObj.Object;
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

	ConstructorHelpers::FObjectFinder<UInputAction> ShowPauseMenuActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_ShowPauseMenu.IA_ShowPauseMenu"));
	if (ShowPauseMenuActionObj.Succeeded())
	{
		ShowPauseMenuAction = ShowPauseMenuActionObj.Object;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> FinalScoreBoardWidgetRef(TEXT("/Game/ProjectG/UI/WBP_PGFinalScoreBoardWidget.WBP_PGFinalScoreBoardWidget_C"));
	if (FinalScoreBoardWidgetRef.Class != nullptr)
	{
		FinalScoreBoardWidgetClass = FinalScoreBoardWidgetRef.Class;
	}

	ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuWidgetRef(TEXT("/Game/ProjectG/UI/WBP_PGPauseMenuWidget.WBP_PGPauseMenuWidget_C"));
	if (PauseMenuWidgetRef.Class != nullptr)
	{
		PauseMenuWidgetClass = PauseMenuWidgetRef.Class;
	}

	OriginalPlayerCharacter = nullptr;
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

void APGPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	
	if (!HasAuthority()) return;

	// On travel first try success
	UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::PostSeamlessTravel: [%s] travel success"), *GetNameSafe(this));
	Client_PostSeamlessTravel();
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
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// [NOT COMPLETE]
		//  ���� priority�� 1. ���߿� controller ���� esc ���� ���ؽ�Ʈ�� �߰��� ������ ���� �� ����
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

void APGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGPlayerController, SpectateTargetCharacter);
	DOREPLIFETIME(APGPlayerController, OriginalPlayerCharacter);
}

void APGPlayerController::Client_PostSeamlessTravel_Implementation()
{
	if (!IsLocalController()) return;

	if (UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance()))
	{
		if (GI->CheckIsTimerActive())
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::Client_PostSeamlessTravel: [%s] stop travel timer"), *GetNameSafe(this));
			GI->NotifyTravelSuccess();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PGPlayerController::Client_PostSeamlessTravel: [%s] travel success but already timeout"), *GetNameSafe(this));
		}
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

void APGPlayerController::InitFinalScoreBoardWidget()
{
	if (IsLocalController() && FinalScoreBoardWidgetClass)
	{
		// clear current viewport
		if (UGameViewportClient* ViewPort = GetWorld()->GetGameViewport())
		{
			ViewPort->RemoveAllViewportWidgets();
		}

		//APGHUD* HUD = Cast<APGHUD>(GetHUD());
		//HUD->InitFinalScoreBoardWidget()

		FinalScoreBoardWidgetInstance = CreateWidget<UPGFinalScoreBoardWidget>(this, FinalScoreBoardWidgetClass);
		if (FinalScoreBoardWidgetInstance)
		{
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());

			UE_LOG(LogTemp, Log, TEXT("PC::InitFinalScoreBoardWidget: FinalScoreBoardWidget created successfully."));
			FinalScoreBoardWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("PC::InitFinalScoreBoardWidget: FinalScoreBoardWidget added to viewport."));

			FinalScoreBoardWidgetInstance->BindPlayerEntry(this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC::InitFinalScoreBoardWidget: Failed to create FinalScoreBoardWidget"));
		}
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

void APGPlayerController::Client_ForceReturnToLobby_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("PGPC::Client_ForceReturnToLobby: Received command from host to leave session"));

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->LeaveSessionAndReturnToLobby();
	}
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
	// �̹� ���� ��忡 ������ �ִٸ� (APGSpectatorPawn�� �����ϰ� �ִٸ�) ����
	if (Cast<APGSpectatorPawn>(GetPawn()))
	{
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_EnterSpectatorMode_Implementation: Already in spectator mode. Ignoring request."));
		return;
	}

	APawn* PrevPawn = GetPawn();
	if (PrevPawn)
	{
		// ���� Pawn�� �Է� ��Ȱ��ȭ
		PrevPawn->DisableInput(this);

		OriginalPlayerCharacter = Cast<APGPlayerCharacter>(PrevPawn);
		UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode_Implementation: OriginalPlayerCharacter set to: %s"), *GetNameSafe(OriginalPlayerCharacter));
	}

	//
	// ---------- Spawn SpectatorPawn -----------------
	//
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true; // ���� ���� �� ũ���� ����

	// ���� Pawn�� ��ġ���� SpectatorPawn ����
	FVector SpawnLoc = PrevPawn ? PrevPawn->GetActorLocation() : FVector::ZeroVector;
	FRotator SpawnRot = PrevPawn ? PrevPawn->GetActorRotation() : FRotator::ZeroRotator;

	// ���� ���� ���� SpectatorPawn�� �ִٸ� �ı�
	if (ControlledSpectator)
	{
		ControlledSpectator->Destroy();
		ControlledSpectator = nullptr;
	}

	ControlledSpectator = GetWorld()->SpawnActor<APGSpectatorPawn>(APGSpectatorPawn::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);

	// ControlledSpectataor�� ���� ��� ĳ���͵� �߰�.
	// ������ �ٸ� ����� ���ٸ� (�ַ��÷����� ���) ���� ��� ���� ����
	if (!ControlledSpectator->InitCachedAllPlayableCharacters(OriginalPlayerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Cannot enter spectator mode. No other players to spectate."));
		return;
	}

	if (ControlledSpectator)
	{
		Possess(ControlledSpectator);

		ControlledSpectator->Server_SetSpectateTarget(true);

		if (SpectateTargetCharacter)
		{
			// SetTarget�� Server_ChangeSpectateTarget_Implementation���� ����
			UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode: Spectator set initial target to %s."), *GetNameSafe(SpectateTargetCharacter));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC::Server_EnterSpectatorMode: No SpectateTargetCharacter found after initial change attempt!"));
		}

		// **�������� SpectatorPawn�� Possess**
		// �� Possess�� ControlledSpectator�� Ŭ���̾�Ʈ�� ����ȭ�ϰ�,
		// Ŭ���̾�Ʈ PlayerController�� GetPawn()�� SpectatorPawn�� ��ȯ�ϰ� �մϴ�.
		UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode: Spectator mode entered and possessed APGSpectatorPawn for %s."), *GetNameSafe(this));

		// Ŭ���̾�Ʈ�� ControlledSpectator�� TargetToOrbit�� �����ϵ��� ����մϴ�.
		// Ŭ���̾�Ʈ���� �ڽ��� SpectatorPawn�� ���� ����� �����϶�� ����.
		// Client_PossessSpectatorPawn�� SpectatorPawn�� ������ �� ȣ��Ǿ�� �����մϴ�.
		// ControlledSpectator�� ������ �� OnRep_SpectateTargetCharacter���� ó���ϴ� ���� �� �����ϴ�.

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn APGSpectatorPawn."));
	}
}

void APGPlayerController::OnRep_SpectateTargetCharacter()
{
	if (IsLocalController() && IsValid(SpectateTargetCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_SpectateTargetCharacter: Target received: %s."), *GetNameSafe(SpectateTargetCharacter));

		APGSpectatorPawn* CurrentSpectator = Cast<APGSpectatorPawn>(GetPawn()); // ���� Possess�� SpectatorPawn ��������
		if (IsValid(CurrentSpectator))
		{
			//CurrentSpectator->SetTargetActor(SpectateTargetCharacter); // Ŭ���̾�Ʈ �� SpectatorPawn�� ��� ����
			//CurrentSpectator->UpdateSpectatorPositionAndRotation(); // �ʱ� ��ġ �� ȸ�� ������Ʈ
			UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_SpectateTargetCharacter: ControlledSpectator valid and target set."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_SpectateTargetCharacter: ControlledSpectator is not yet valid. (Possibly not Possessed by server yet)"));
			// �� ��� ControlledSpectator�� Possess�Ǵ� ������ ��ٷ��� �մϴ�.
			// �Ǵ� ControlledSpectator ������ ReplicatedUsing���� �����, �ش� OnRep���� ó���� ���� �ֽ��ϴ�.
			// ������ GetPawn()�� PlayerState�� PlayerPawn ������ ���� Ŭ���̾�Ʈ�� ����ȭ�ǹǷ� ���� ���� �����ϴ�.
		}
	}
	else if (IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_SpectateTargetCharacter: SpectateTargetCharacter is nullptr or not local controller."));
	}
}

void APGPlayerController::OnSpectateNext(const FInputActionValue& Value)
{
	if (!IsLocalController()) return;
	if (Cast<APGSpectatorPawn>(GetPawn()))
	{
		Server_ChangeSpectateTarget(true);
		UE_LOG(LogTemp, Log, TEXT("PC::OnSpectateNext: Client requested next spectate target."));
	}
}

void APGPlayerController::OnSpectatePrev(const FInputActionValue& Value)
{
	if (!IsLocalController()) return;
	if (Cast<APGSpectatorPawn>(GetPawn()))
	{
		Server_ChangeSpectateTarget(false);
		UE_LOG(LogTemp, Log, TEXT("PC::OnSpectatePrev: Client requested previous spectate target."));
	}
}

void APGPlayerController::OnShowPauseMenu(const FInputActionValue& Value)
{
	if (!IsLocalController()) return;

	if (FinalScoreBoardWidgetInstance && FinalScoreBoardWidgetInstance->IsInViewport()) return;

	if (PauseMenuWidgetClass)
	{
		PauseMenuWidgetInstance = CreateWidget<UPGPauseMenuWidget>(this, PauseMenuWidgetClass);
		if (PauseMenuWidgetInstance)
		{
			UE_LOG(LogTemp, Log, TEXT("PC::OnShowPauseMenu: PauseMenuWidget created successfully"));

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());

			PauseMenuWidgetInstance->AddToViewport();
			PauseMenuWidgetInstance->Init(this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC::OnShowPauseMenu: Failed to create PauseMenuWidget"))
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PC::OnShowPauseMenu: No PauseMenuWidget Class"));
	}
}

void APGPlayerController::Server_ChangeSpectateTarget_Implementation(bool bNext)
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// ���� ���� ����� �ִ� ��� �ش� ����� index ����
	int32 CurrentTargetIndex = INDEX_NONE;
	if (IsValid(SpectateTargetCharacter))
	{
		CurrentTargetIndex = CachedAllPlayableCharacters.IndexOfByKey(SpectateTargetCharacter);
	}

	// ���ο� ������� index
	int32 NewTargetIndex = 0;

	// ���� ���� ���� ���, �Է¿� ���� ���� ��� ���� Ȥ�� ���� ĳ������ �ε��� ����
	// ���� ������ �����ϴ� ���, �迭���� ���� ����� ã�� �ʰ�, �迭�� 0�� ĳ���� ����
	if (CurrentTargetIndex != INDEX_NONE) 
	{
		if (bNext)
		{
			NewTargetIndex = (CurrentTargetIndex + 1) % CachedAllPlayableCharacters.Num();
		}
		else
		{
			NewTargetIndex = (CurrentTargetIndex - 1 + CachedAllPlayableCharacters.Num()) % CachedAllPlayableCharacters.Num();
		}
	}

	ACharacter* NewTargetCandidate = CachedAllPlayableCharacters[NewTargetIndex];
	
	// ���ο� ����� ���� ���� ������ �ٸ� ��쿡�� ������Ʈ
	// ���ο� ����� ���� ���� ���� ��� => ���� ������ ĳ���Ͱ� ���ʿ� �� �� �ۿ� ������ ��� ������Ʈ x
	if (NewTargetCandidate != SpectateTargetCharacter)
	{
		SpectateTargetCharacter = NewTargetCandidate;
		UE_LOG(LogTemp, Log, TEXT("PC::Server_ChangeSpectateTarget_Implementation: Changed spectate target to: %s"), *GetNameSafe(SpectateTargetCharacter));

		if (IsValid(ControlledSpectator) && IsValid(SpectateTargetCharacter))
		{
			ControlledSpectator->SetTargetActor(SpectateTargetCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC::Server_ChangeSpectateTarget_Implementation: ControlledSpectator or SpectateTargetCharacter invalid after target change."));
		}
	}
	else
	{
		// �� �޽����� AllPlayableCharacters.Num() == 1 �� ���� ��� (���� ������ ĳ���Ͱ� ���ʿ� �� �� �ۿ� ������ ���)
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_ChangeSpectateTarget_Implementation: Spectate target remains the same (no other valid unique target)."));
	}
}

// ������ �Է¿� ���� ȸ�� ó��
void APGPlayerController::OnOrbitYaw(const FInputActionValue& Value)
{
	// Ŭ���̾�Ʈ���� �Է� ó��
	if (!IsLocalController()) return;

	float AxisValue = Value.Get<float>();

	// ���� �÷��̾� ��Ʈ�ѷ��� APGSpectatorPawn�� Possess�ϰ� ���� ���� ó��
	if (APGSpectatorPawn* Spectator = Cast<APGSpectatorPawn>(GetPawn()))
	{
		Spectator->UpdateOrbitYawInput(AxisValue);
	}
}

/*
* On host or client server travel failed
* call server RPC to redo server travel
* server PC Set travel failed flag on GM
*/
void APGPlayerController::NotifyTravelFailed()
{
	if (!IsLocalController()) return;

	UE_LOG(LogTemp, Warning, TEXT("PGPC::NotifyTravelFailed: [%s] Report travel failed to server."), *GetNameSafe(this));
	Server_ReportTravelFailed();
}

void APGPlayerController::Server_ReportTravelFailed_Implementation()
{
	if (!HasAuthority()) return;

	APGGameMode* GM = Cast<APGGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	UE_LOG(LogTemp, Warning, TEXT("PGPC::Server_ReportTravelFailed: Report travel failed to GM."));
	GM->SetIsTravelFailedExist();
}
