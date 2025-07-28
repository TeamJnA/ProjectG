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

#include "Blueprint/UserWidget.h"
#include "UI/PGFinalScoreBoardWidget.h"
#include "UI/PGHUD.h"

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

APGPlayerController::APGPlayerController()
{
	ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextObj(TEXT("/Game/ProjectG/Character/Input/IMC_Default.IMC_Default"));
	if (MappingContextObj.Succeeded())
	{
		DefaultMappingContext = MappingContextObj.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> SpectateActionObj(TEXT("/Game/ProjectG/Character/Input/Actions/IA_Spectate.IA_Spectate"));
	if (SpectateActionObj.Succeeded())
	{
		SpectateAction = SpectateActionObj.Object;
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

	static ConstructorHelpers::FClassFinder<UUserWidget> ScoreBoardWidgetRef(TEXT("/Game/ProjectG/UI/WBP_PGFinalScoreBoardWidget.WBP_PGFinalScoreBoardWidget_C"));
	if (ScoreBoardWidgetRef.Class != nullptr)
	{
		ScoreBoardWidgetClass = ScoreBoardWidgetRef.Class;
	}

	OriginalPlayerCharacter = nullptr;
}

void APGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

	bShowMouseCursor = false;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 1);
	}

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
		EnhancedInputComponent->BindAction(SpectateAction, ETriggerEvent::Started, this, &APGPlayerController::OnSpectate);
		// up, down
		EnhancedInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &APGPlayerController::OnSpectateNext);
		EnhancedInputComponent->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &APGPlayerController::OnSpectatePrev);
		// left, right
		EnhancedInputComponent->BindAction(OrbitYawAction, ETriggerEvent::Triggered, this, &APGPlayerController::OnOrbitYaw);
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

void APGPlayerController::OnSpectate(const FInputActionValue& Value)
{
	// 이 함수는 항상 로컬 클라이언트에서 실행됩니다.
	// 여기서 서버 RPC를 호출하여 서버에 관전 모드 진입을 요청합니다.
	if (!IsLocalController()) return;
	Server_EnterSpectatorMode();

	UE_LOG(LogTemp, Warning, TEXT("PC: OnSpectateActionTriggered: Client requested EnterSpectatorMode."));
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
	if (IsLocalController() && ScoreBoardWidgetClass)
	{
		// clear current viewport
		if (UGameViewportClient* ViewPort = GetWorld()->GetGameViewport())
		{
			ViewPort->RemoveAllViewportWidgets();
		}

		//APGHUD* HUD = Cast<APGHUD>(GetHUD());
		//HUD->InitFinalScoreBoardWidget()

		ScoreBoardWidgetInstance = CreateWidget<UPGFinalScoreBoardWidget>(this, ScoreBoardWidgetClass);
		if (ScoreBoardWidgetInstance)
		{
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());

			UE_LOG(LogTemp, Log, TEXT("PC::InitFinalScoreBoardWidget: FinalScoreBoardWidget created successfully."));
			ScoreBoardWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("PC::InitFinalScoreBoardWidget: FinalScoreBoardWidget added to viewport."));

			ScoreBoardWidgetInstance->BindPlayerEntry(this);
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
	// 서버에서만 실행
	if (!HasAuthority()) return;

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
		// 기존 Pawn의 충돌 비활성화 (필요하다면)
		// PrevPawn->SetActorEnableCollision(false);

		OriginalPlayerCharacter = Cast<ACharacter>(PrevPawn);
		UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode_Implementation: OriginalPlayerCharacter set to: %s"), *GetNameSafe(OriginalPlayerCharacter));
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 관전 가능한 캐릭터들을 Iterator를 돌아 채우기
	// 만약 솔로플레이인 경우 관전모드에 2회 이상 진입 시도할 수있기 때문에, IsSpectateTargetChached 플래그를 따로 두어 진입 차단
	if (!IsSpectateTargetCached)
	{
		for (TActorIterator<ACharacter> It(World); It; ++It)
		{
			ACharacter* CurrentChar = *It;

			if (CurrentChar && CurrentChar != OriginalPlayerCharacter)
			{
				CachedAllPlayableCharacters.Add(CurrentChar);
			}
		}

		IsSpectateTargetCached = true;
	}

	// 관전할 다른 대상이 없다면 (솔로플레이인 경우) 관전 모드 진입 방지
	if (CachedAllPlayableCharacters.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Cannot enter spectator mode. No other players to spectate."));
		return;
	}

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

	if (ControlledSpectator)
	{
		Possess(ControlledSpectator);

		// 서버에서 관전 대상 찾기
		//SpectateTargetCharacter = Cast<ACharacter>(FindSpectateTarget());

		Server_ChangeSpectateTarget(true);

		if (SpectateTargetCharacter)
		{
			// SetTarget은 Server_ChangeSpectateTarget_Implementation에서 수행
			UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode: Spectator set initial target to %s."), *GetNameSafe(SpectateTargetCharacter));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PC::Server_EnterSpectatorMode: No SpectateTargetCharacter found after initial change attempt!"));
		}

		// **서버에서 SpectatorPawn을 Possess**
		// 이 Possess가 ControlledSpectator를 클라이언트에 동기화하고,
		// 클라이언트 PlayerController의 GetPawn()이 SpectatorPawn을 반환하게 합니다.
		UE_LOG(LogTemp, Log, TEXT("PC::Server_EnterSpectatorMode: Spectator mode entered and possessed APGSpectatorPawn for %s."), *GetNameSafe(this));

		// 클라이언트에 ControlledSpectator에 TargetToOrbit을 설정하도록 명령합니다.
		// 클라이언트에게 자신의 SpectatorPawn에 관전 대상을 설정하라고 지시.
		// Client_PossessSpectatorPawn은 SpectatorPawn이 복제된 후 호출되어야 안전합니다.
		// ControlledSpectator가 복제된 후 OnRep_SpectateTargetCharacter에서 처리하는 것이 더 낫습니다.

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

		APGSpectatorPawn* CurrentSpectator = Cast<APGSpectatorPawn>(GetPawn()); // 현재 Possess된 SpectatorPawn 가져오기
		if (IsValid(CurrentSpectator))
		{
			//CurrentSpectator->SetTargetActor(SpectateTargetCharacter); // 클라이언트 측 SpectatorPawn에 대상 설정
			//CurrentSpectator->UpdateSpectatorPositionAndRotation(); // 초기 위치 및 회전 업데이트
			UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_SpectateTargetCharacter: ControlledSpectator valid and target set."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_SpectateTargetCharacter: ControlledSpectator is not yet valid. (Possibly not Possessed by server yet)"));
			// 이 경우 ControlledSpectator가 Possess되는 시점을 기다려야 합니다.
			// 또는 ControlledSpectator 변수를 ReplicatedUsing으로 만들고, 해당 OnRep에서 처리할 수도 있습니다.
			// 하지만 GetPawn()은 PlayerState의 PlayerPawn 복제에 의해 클라이언트로 동기화되므로 보통 문제 없습니다.
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

void APGPlayerController::Server_ChangeSpectateTarget_Implementation(bool bNext)
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 현재 관전 대상이 있는 경우 해당 대상의 index 저장
	int32 CurrentTargetIndex = INDEX_NONE;
	if (IsValid(SpectateTargetCharacter))
	{
		CurrentTargetIndex = CachedAllPlayableCharacters.IndexOfByKey(SpectateTargetCharacter);
	}

	// 새로운 관전대상 index
	int32 NewTargetIndex = 0;

	// 현재 관전 중인 경우, 입력에 따라 관전 대상 다음 혹은 이전 캐릭터의 인덱스 저장
	// 최초 관전을 시작하는 경우, 배열에서 관전 대상을 찾지 않고, 배열의 0번 캐릭터 관전
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
	
	// 새로운 대상이 현재 대상과 실제로 다를 경우에만 업데이트
	// 새로운 대상이 현재 대상과 같은 경우 => 관전 가능한 캐릭터가 애초에 한 명 밖에 없었을 경우 업데이트 x
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
		// 이 메시지는 AllPlayableCharacters.Num() == 1 일 때만 출력 (관전 가능한 캐릭터가 애초에 한 명 밖에 없었을 경우)
		UE_LOG(LogTemp, Warning, TEXT("PC::Server_ChangeSpectateTarget_Implementation: Spectate target remains the same (no other valid unique target)."));
	}
}

// 관전자 입력에 따라 회전 처리
void APGPlayerController::OnOrbitYaw(const FInputActionValue& Value)
{
	// 클라이언트에서 입력 처리
	if (!IsLocalController()) return;

	float AxisValue = Value.Get<float>();

	// 현재 플레이어 컨트롤러가 APGSpectatorPawn을 Possess하고 있을 때만 처리
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
