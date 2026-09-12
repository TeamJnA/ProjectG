// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGAmbientSoundComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "PGAmbientSoundComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Game/PGGameState.h"
#include "Utils/PGEnemyRegistry.h"
#include "PGLogChannels.h"

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
}

void UPGAmbientSoundComponent::InitTargetActors()
{
	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("UPGAmbientSoundComponent::InitTargetActors"));
	InitTrackedEnemies();
	InitTrackedPlayerCharacters();
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

	const APGGameState* GameState = GetWorld() ? GetWorld()->GetGameState<APGGameState>() : nullptr;
	if (!GameState)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();
	TrackedPlayerCharacters.Reserve(GameState->PlayerArray.Num());

	for (const TObjectPtr<APlayerState>& PlayerState : GameState->PlayerArray)
	{
		APGPlayerCharacter* PlayerCharacter = PlayerState ? Cast<APGPlayerCharacter>(PlayerState->GetPawn()) : nullptr;
		if (IsValid(PlayerCharacter) && PlayerCharacter != OwnerActor)
		{
			TrackedPlayerCharacters.AddUnique(PlayerCharacter);
		}
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

	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("UpdateAwareness"));

	// Enemy 관련 작업(거리 / 발견)
	const FVector OwnerLocation = OwnerPawn->GetActorLocation();

	for (int32 Index = TrackedEnemies.Num() - 1; Index >= 0; --Index)
	{
		AActor* Enemy = TrackedEnemies[Index].Get();
		if (!IsValid(Enemy))
		{
			EnemyDistances.Remove(TrackedEnemies[Index]);
			TrackedEnemies.RemoveAtSwap(Index);
			continue;
		}

		// 거리 구하기
		EnemyDistances.FindOrAdd(Enemy) = FVector::Distance(OwnerLocation, Enemy->GetActorLocation());

		// 에너미 화면에 있는 지 확인
		if (!SeenEnemies.Contains(Enemy) && IsActorVisibleOnScreen(Enemy, PlayerController))
		{
			UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Find Enemy %s"), *Enemy->GetName());
			SeenEnemies.Add(Enemy);
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

void UPGAmbientSoundComponent::OnAmbientSoundTrigger()
{
	UE_LOG(LogPGAmbientSoundComponent, Log, TEXT("Play Ambient Sound"));

	/*
	* TODO : 사운드 재생을 하되, 재생 중인지... 최근에 재생했는지... 여부를 확인할 것...!
	*/

}

bool UPGAmbientSoundComponent::IsActorVisibleOnScreen(
	const AActor* TargetActor,
	APlayerController* PlayerController) const
{
	if (!IsValid(TargetActor) || !IsValid(PlayerController))
	{
		return false;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	FVector2D ScreenPosition;
	if (!PlayerController->ProjectWorldLocationToScreen(TargetLocation, ScreenPosition, true))
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

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AmbientSoundVisibility), true);
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;
	const bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(
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

	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(DeadTag);
}

/*
// Called every frame
void UPGAmbientSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/