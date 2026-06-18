// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGM_Test.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/PGCharacterBase.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Item/PGItemActor.h"

#include "Level/Misc/PGDoor1.h"

#include "Kismet/GameplayStatics.h"

#include "GameFramework/GameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"

APGGM_Test::APGGM_Test()
{
	static ConstructorHelpers::FClassFinder<APGGhostCharacter> GhostPawnBPClass(TEXT("/Game/ProjectG/Enemy/Ghost/Character/BP_GhostCharacter.BP_GhostCharacter_C"));
	if (GhostPawnBPClass.Class != nullptr)
	{
		GhostCharacterClass = GhostPawnBPClass.Class;
	}
}

APGSoundManager* APGGM_Test::GetSoundManager()
{
	return SoundManager;
}

void APGGM_Test::SpawnDoorTEST(FVector InLocation, FRotator InRotator)
{
	const FVector SpawnLocation = InLocation;
	const FRotator SpawnRotation = InRotator;
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true;

	check(PGDoor);
	APGDoor1* DefaultDoor = PGDoor->GetDefaultObject<APGDoor1>();
	if (DefaultDoor)
	{
		DefaultDoor->SpawnDoor(GetWorld(), PGDoor, SpawnTransform, SpawnParams, false);
	}
}

void APGGM_Test::BreakDoorTEST(AActor* InteractInvestigator)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 1. 해당 클래스의 모든 액터를 담을 배열 생성
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, APGDoor1::StaticClass(), FoundActors);

	// 2. 배열을 순회하며 함수 실행
	for (AActor* Actor : FoundActors)
	{
		APGDoor1* Door = Cast<APGDoor1>(Actor);
		if (Door)
		{
			// 3. APGDoor1 클래스 내부의 A 함수 실행
			Door->BreakDoorByEnemy(InteractInvestigator);

			// 만약 레벨에 문이 여러 개인데 하나만 실행하고 싶다면 여기서 break;
			// break; 
		}
	}
}

void APGGM_Test::SpawnTestItem(FName InName, FVector InVector, FRotator InRotator)
{
	UWorld* World = GetWorld();
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI || !World)
	{
		return;
	}

	const FTransform SpawnTransform(InRotator, InVector);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GI->RequestLoadItemData(InName, FOnItemDataLoaded::CreateLambda([World, SpawnTransform, SpawnParams,  this](UPGItemData* LoadedItemData)
		{
			if (LoadedItemData)
			{
				APGItemActor* NewItem = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
				if (NewItem)
				{
					NewItem->InitWithData(LoadedItemData);
				}
			}
		}));
}

void APGGM_Test::RotateItem(AActor* Item)
{
	if (APGItemActor* PGItem = Cast<APGItemActor>(Item))
	{
		if (IsValid(PGItem->GetStaticMeshComponent()))
		{
			FTransform CurrentTransform = PGItem->GetActorTransform();

			FVector Min, Max;
			PGItem->GetStaticMeshComponent()->GetLocalBounds(Min, Max);

			FVector LocalCenter = (Min + Max) * 0.5f;
			FVector LocalExtents = (Max - Min) * 0.5f;

			// 로컬 기준 '오른쪽' 오프셋 (+Y 방향)
			FVector LocalRightCenter = FVector(LocalCenter.X, LocalCenter.Y - LocalExtents.Y, LocalCenter.Z);

			FRotator LayDownRotation = FRotator(0.0f, 0.0f, -90.0f);
			FQuat NewRotation = CurrentTransform.GetRotation() * LayDownRotation.Quaternion();
			CurrentTransform.SetRotation(NewRotation);

			// 월드 오프셋 변환 및 위치 역산
			FVector WorldOffset = CurrentTransform.GetRotation().RotateVector(LocalRightCenter);
			FVector AdjustedLocation = CurrentTransform.GetLocation() - WorldOffset;
			CurrentTransform.SetLocation(AdjustedLocation);

			// 계산된 최종 위치와 회전을 액터에 덮어씌웁니다.
			PGItem->SetActorTransform(CurrentTransform);
		}
	}
}

void APGGM_Test::RespawnPlayer(AController* DeadPlayerController, const FTransform& SpawnTransform)
{
	APGPlayerController* DeadPC = Cast<APGPlayerController>(DeadPlayerController);
	if (!DeadPC)
	{
		UE_LOG(LogTemp, Error, TEXT("GM::RespawnPlayer called with a NULL controller or NULL player state"));
		return;
	}

	APawn* OldPawn = DeadPC->GetPawn();
	if (!OldPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("GM::RespawnPlayer: OldPawn was null, proceeding to spawn new character."))
	}
	DeadPC->UnPossess();
	OldPawn->Destroy();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APGPlayerCharacter* NewCharacter = GetWorld()->SpawnActor<APGPlayerCharacter>(DefaultPawnClass, SpawnTransform, SpawnParams);
	if (!NewCharacter || !SoundManager)
	{
		UE_LOG(LogTemp, Error, TEXT("GM::RespawnPlayer: No character or sound manager valid"));
		return;
	}
	DeadPC->Client_OnRevive();
	DeadPC->Possess(NewCharacter);
	NewCharacter->InitSoundManager(SoundManager);
	NewCharacter->OnRevive();
}

void APGGM_Test::BeginPlay()
{
	Super::BeginPlay();

	// Spawn sound manager on the lever.
	SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
	if (SoundManager) {
		UE_LOG(LogTemp, Log, TEXT("Sound manager spawn complete."));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APGGM_Test::InitWorld, 4.0f, false);
}

void APGGM_Test::InitWorld()
{
	InitSoundManagerToPlayers();
	//SpawnGhost();
}

void APGGM_Test::InitSoundManagerToPlayers()
{
	// Set the character's SoundManager pointer to the globally spawned soundmanager instance.
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APGCharacterBase::StaticClass(), AllCharacters);

	for (AActor* Character : AllCharacters)
	{
		APGCharacterBase* PGCB = Cast<APGCharacterBase>(Character);
		if (PGCB)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s [2nd]"), *PGCB->GetName());
			PGCB->InitSoundManager(SoundManager);
		}
	}

	TArray<AActor*> AllGimmicks;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APGTriggerGimmickBase::StaticClass(), AllGimmicks);

	for (AActor* Gmick : AllGimmicks)
	{
		APGTriggerGimmickBase* PGG = Cast<APGTriggerGimmickBase>(Gmick);
		if (PGG)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s [2nd]"), *PGG->GetName());
			PGG->InitSoundManager();
		}
	}
}

void APGGM_Test::SpawnGhost()
{
	UE_LOG(LogTemp, Log, TEXT("GM::SpawnGhostsForPlayers: Spawning ghosts for all players."));

	AGameStateBase* GS = GetGameState<AGameStateBase>();
	if (!GS)
	{
		UE_LOG(LogTemp, Error, TEXT("APGGM_Test::SpawnGhostsForPlayers: No APGGameState found!"));
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

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS && PS->GetPawn())
		{
			const FVector PlayerLocation = PS->GetPawn()->GetActorLocation();
			const FVector SpawnLocation = PlayerLocation + FVector(300.0f, 0.0f, 100.0f);
			const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

			APGGhostCharacter* NewGhost = GetWorld()->SpawnActor<APGGhostCharacter>(GhostCharacterClass, SpawnTransform, SpawnParams);
			if (NewGhost)
			{
				NewGhost->SetTargetPlayerState(PS);

				UE_LOG(LogTemp, Log, TEXT("APGGameMode: Spawned Ghost (%s) and assigned to Player (%s)"), *NewGhost->GetName(), *PS->GetPlayerName());
			}
		}
	}
}