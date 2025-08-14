// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGM_Test.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/PGCharacterBase.h"
#include "Sound/PGSoundManager.h"
#include "Kismet/GameplayStatics.h"

#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"

APGGM_Test::APGGM_Test()
{
	// 바닥이랑 에너미캐릭터 둘 다 생성자 설정
	static ConstructorHelpers::FClassFinder<AActor> GroundRef(TEXT("/Game/ProjectG/Levels/TEST_NavMesh/GroundAndNavMeshBound.GroundAndNavMeshBound_C"));
	if (GroundRef.Class)
	{
		Ground = GroundRef.Class;
	}
	static ConstructorHelpers::FClassFinder<APGBlindCharacter> EnemyRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/BP_BlindCharacter.BP_BlindCharacter_C"));
	if (EnemyRef.Class)
	{
		EnemyCharacter = EnemyRef.Class;
	}
}

APGSoundManager* APGGM_Test::GetSoundManager()
{
	return SoundManager;
}

void APGGM_Test::BeginPlay()
{
	// Spawn sound manager on the lever.
	SoundManager = GetWorld()->SpawnActor<APGSoundManager>(APGSoundManager::StaticClass(), FVector(0.0f, 0.0f, -500.0f), FRotator::ZeroRotator);
	if (SoundManager) {
		UE_LOG(LogTemp, Log, TEXT("Sound manager spawn complete."));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn sound manager."));
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APGGM_Test::InitSoundManagerToPlayers, 4.0f, false);
}

void APGGM_Test::InitSoundManagerToPlayers()
{
	// Set the character's SoundManager pointer to the globally spawned soundmanager instance.
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		APlayerController* PC = it->Get();
		if (!PC) continue;

		APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(PC->GetPawn());
		if (PGPC)
		{
			UE_LOG(LogTemp, Log, TEXT("Init sound manager to %s"), *PGPC->GetName());
			PGPC->InitSoundManager(SoundManager);
		}
	}


	// Enemy 찾아서 InitSoundManager하고 Test 할 것
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
}

void APGGM_Test::TEST_MakeGroundSpawnMonsterAndBuildNavMesh(FVector MonsterSpawnLocation, FVector GroundSpawnLocation, FVector GroundScale)
{
	// 바닥이랑 몬스터 스폰할 것.

	// 바닥
	FVector SpawnLocation = GroundSpawnLocation;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	GetWorld()->SpawnActor<AActor>(Ground, SpawnLocation, SpawnRotation);

	// 바닥의 scale 설정
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	SpawnTransform.SetScale3D(GroundScale);

	AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(
		Ground,         // 생성할 액터 클래스
		SpawnTransform,         // 위치, 회전, 스케일 포함
		nullptr,                // 소유자
		nullptr,                // 인스티게이터
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (SpawnedActor)
	{
		// 필요한 초기화 코드 (옵션)
		UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);
	}

	// 몬스터
	FVector MonSpawnLocation = MonsterSpawnLocation;
	FRotator MonSpawnRotation = FRotator::ZeroRotator;
	GetWorld()->SpawnActor<APGBlindCharacter>(EnemyCharacter, MonSpawnLocation, MonSpawnRotation);

	// Spawn and Build Nav Mesh.
	ANavMeshBoundsVolume* NewNavMeshVolume = GetWorld()->SpawnActor<ANavMeshBoundsVolume>(ANavMeshBoundsVolume::StaticClass(), 
		GroundSpawnLocation, 
		FRotator::ZeroRotator);
	if (!NewNavMeshVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn NewNavMeshVolume."));
		return;
	}
	NewNavMeshVolume->SetActorScale3D(FVector(100.0, 100.0, 100.0));

	// NewNavMeshVolume->GetRootComponent()->Bounds = FBox(-10.0, 10.0);

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		//NavSys->Build();
		//NavSys->ReleaseInitialBuildingLock();
		UE_LOG(LogTemp, Log, TEXT("Start build navmesh."));
		NavSys->OnNavigationBoundsUpdated(NewNavMeshVolume);
		NavSys->Build();
		// Navigation System에서 Initial Building Lock 확인
	}
}