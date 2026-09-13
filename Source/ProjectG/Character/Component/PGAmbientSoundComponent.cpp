// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGAmbientSoundComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "PGAmbientSoundComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Game/PGGameState.h"
#include "Utils/PGEnemyRegistry.h"
#include "PGLogChannels.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UPGAmbientSoundComponent::UPGAmbientSoundComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UPGAmbientSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	TryBindEnterSequenceFinishDelegate();
}

void UPGAmbientSoundComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APGGameState* GS = GetWorld() ? GetWorld()->GetGameState<APGGameState>() : nullptr)
	{
		GS->OnLocalEnterSequenceFinished.RemoveAll(this);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			AwarenessUpdateTimerHandle
		);
	}

	if (IsValid(AmbientAudioComponent.Get()))
	{
		AmbientAudioComponent->Stop();
		AmbientAudioComponent->DestroyComponent();
	}

	AmbientAudioComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UPGAmbientSoundComponent::TryBindEnterSequenceFinishDelegate()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	if (bEnterSequenceBinded)
	{
		return;
	}
	bEnterSequenceBinded = true;

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	if (!GS)
	{
		return;
	}

	GS->OnLocalEnterSequenceFinished.AddUObject(
		this,
		&UPGAmbientSoundComponent::InitTargetActors
	);

	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Bind to Sequence completely"));
}

void UPGAmbientSoundComponent::InitTargetActors()
{
	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("UPGAmbientSoundComponent::InitTargetActors"));
	InitTrackedEnemies();
	InitTrackedPlayerCharacters();

	InitAmbientAudioComponent();

	StartAwarenessUpdates();
}

void UPGAmbientSoundComponent::InitTrackedEnemies()
{
	TrackedEnemies.Reset();

	UWorld* World = GetWorld();
	UPGEnemyRegistry* EnemyRegistry = World ? World->GetSubsystem<UPGEnemyRegistry>() : nullptr;
	if (!EnemyRegistry)
	{
		return;
	}

	const TArray<TWeakObjectPtr<AActor>>& RegisteredEnemies = EnemyRegistry->GetEnemies();
	TrackedEnemies.Reserve(RegisteredEnemies.Num());

	for (const TWeakObjectPtr<AActor>& Enemy : RegisteredEnemies)
	{
		if (Enemy.IsValid())
		{
			TrackedEnemies.Add(Enemy);
		}
	}
}

void UPGAmbientSoundComponent::InitTrackedPlayerCharacters()
{
	TrackedPlayerCharacters.Reset();

	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState<AGameStateBase>() : nullptr;
	if (!GameState)
	{
		UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Cannot find gamestate in InitTrackedPlayerCharacters"));
		return;
	}

	const AActor* OwnerActor = GetOwner();
	TrackedPlayerCharacters.Reserve(GameState->PlayerArray.Num());

	for (const TObjectPtr<APlayerState>& PlayerState : GameState->PlayerArray)
	{
		APGPlayerCharacter* PlayerCharacter = PlayerState ? Cast<APGPlayerCharacter>(PlayerState->GetPawn()) : nullptr;
		if (IsValid(PlayerCharacter) && PlayerCharacter != OwnerActor)
		{
			UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Add Player Characters to track list"));
			TrackedPlayerCharacters.AddUnique(PlayerCharacter);
		}
	}

	APawn* Pawn = Cast<APawn>(GetOwner());
	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Player Count : %d, Localled : %d"), int32(TrackedPlayerCharacters.Num()), int32(Pawn->IsLocallyControlled()));
}

void UPGAmbientSoundComponent::InitAmbientAudioComponent()
{
	if (	IsValid(AmbientMetaSound.Get()))
	{
		AmbientAudioComponent = UGameplayStatics::CreateSound2D(
			this,
			AmbientMetaSound.Get(),
			1.0f,   // VolumeMultiplier
			1.0f,   // PitchMultiplier
			0.0f,   // StartTime
			nullptr,
			false,  // bPersistAcrossLevelTransition
			false   // bAutoDestroy: 종료 후에도 재사용
		);

		AmbientAudioComponent->Play();
	}
}

void UPGAmbientSoundComponent::StartAwarenessUpdates()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(AwarenessUpdateTimerHandle);

	World->GetTimerManager().SetTimer(
		AwarenessUpdateTimerHandle,
		this,
		&ThisClass::UpdateAwareness,
		AwarenessUpdateInterval,
		true
	);
}

void UPGAmbientSoundComponent::UpdateAwareness()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!OwnerPawn || !PlayerController)
	{
		return;
	}

	// Enemy 관련 작업(거리 / 발견)
	const FVector OwnerLocation = OwnerPawn->GetActorLocation();

	// Enemy와의 거리를 측정해서 BGM Stress를 늘린다.
	bool bEnemyNear = false;

	for (int32 Index = TrackedEnemies.Num() - 1; Index >= 0; --Index)
	{
		AActor* Enemy = TrackedEnemies[Index].Get();
		if (!IsValid(Enemy))
		{
			TrackedEnemies.RemoveAtSwap(Index);
			continue;
		}

		// 거리 구하기
		float EnemyDistance = FVector::Distance(OwnerLocation, Enemy->GetActorLocation());
		if (EnemyDistance < 2000.0f)
		{
			bEnemyNear = true;
		}

		// 에너미 화면에 있는 지 확인
		if (!SeenEnemies.Contains(Enemy) && IsActorVisibleOnScreen(Enemy, PlayerController))
		{
			UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Find Enemy %s"), *Enemy->GetName());
			SeenEnemies.Add(Enemy);
			OnAmbientSoundTrigger();
		}
	}

	// 주변에 적이 있을 경우, 텐션 수치를 올리며 제한 수치 이상일 시 Tension 재생
	if (bEnemyNear)
	{
		CurTensionValue += 1.0f;
		if (CurTensionValue > BaseTensionThreshold)
		{
			OnAmbientSoundTrigger();
		}
	}

	// 사망한 캐릭터를 처음 발견했을 때
	if (bFirstCorpseDiscovered)
	{
		return;
	}

	for (int32 Index = TrackedPlayerCharacters.Num() - 1; Index >= 0; --Index)
	{
		APGPlayerCharacter* PlayerCharacter = TrackedPlayerCharacters[Index].Get();
		if (!IsValid(PlayerCharacter))
		{
			TrackedPlayerCharacters.RemoveAtSwap(Index);
			UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Target Player Character Not Valid"));
			continue;
		}

		if (IsPlayerDead(PlayerCharacter) && IsActorVisibleOnScreen(PlayerCharacter, PlayerController))
		{
			UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Find Corpse"));

			bFirstCorpseDiscovered = true;
			OnAmbientSoundTrigger();
			break;
		}
	}
}

void UPGAmbientSoundComponent::GimmickTriggerTension()
{
	// 일정 수치 이상일 시 텐션 소리 재생, 아닐 경우 긴장감 수치 증가.
	if (CurTensionValue > GimmickTensionThreshold)
	{
		OnAmbientSoundTrigger();
	}
	else
	{
		CurTensionValue += 30.0f;
	}
}

void UPGAmbientSoundComponent::OnAmbientSoundTrigger()
{
	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Play Ambient Sound"));

	if (!IsValid(AmbientAudioComponent.Get()))
	{
		return;
	}

	if (bIsTensionReady)
	{
		AmbientAudioComponent->SetTriggerParameter(FName("TensionTrigger"));
		CurTensionValue = 0.0f;

		// TensionCooldownTimerHandle후에 Tension Sound 다시 재생 가능.
		bIsTensionReady = false;

		GetWorld()->GetTimerManager().SetTimer(
			TensionCooldownTimerHandle,
			this,
			&UPGAmbientSoundComponent::ResetTensionReady,
			TensionRepeatDelayTime,
			false
		);
	}
	else
	{
		// Tension 쿨이 아직 안왔을 경우, 긴장감 수치를 일정 수치로 유지
		CurTensionValue = 20.0f;
	}
}

bool UPGAmbientSoundComponent::IsActorVisibleOnScreen(
	const AActor* TargetActor,
	APlayerController* PlayerController) const
{
	UWorld* World = GetWorld();
	if (!IsValid(TargetActor) || !IsValid(PlayerController) || !World)
	{
		return false;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TargetLocation = TargetActor->GetActorLocation();

	// 일정 거리 이하만 추적
	if (FVector::DistSquared(CameraLocation, TargetLocation) >
		FMath::Square(MaxVisibilityDistance))
	{
		return false;
	}

	FVector2D ScreenPosition;
	if (!PlayerController->ProjectWorldLocationToScreen(
		TargetLocation, ScreenPosition, true))
	{
		return false;
	}

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);

	if (ViewportWidth <= 0 || ViewportHeight <= 0 ||
		ScreenPosition.X < 0.0f || ScreenPosition.X > ViewportWidth ||
		ScreenPosition.Y < 0.0f || ScreenPosition.Y > ViewportHeight)
	{
		return false;
	}

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(AmbientSoundVisibility), true);
	QueryParams.AddIgnoredActor(GetOwner());

	// 사이에 벽이 없는 경우에만 가능
	FHitResult HitResult;
	const bool bBlockingHit = World->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		TargetLocation,
		ECC_CameraModeTrace,
		QueryParams
	);

	return !bBlockingHit || HitResult.GetActor() == TargetActor;
}

bool UPGAmbientSoundComponent::IsPlayerDead(APGPlayerCharacter* PlayerCharacter) const
{
	static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Player.State.Dead"));

	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(PlayerCharacter);
	const UAbilitySystemComponent* AbilitySystemComponent =	AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;

	const bool bIsDead = AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(DeadTag);

	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(DeadTag);
}

void UPGAmbientSoundComponent::ResetTensionReady()
{
	bIsTensionReady = true;
}

/*
// Called every frame
void UPGAmbientSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/