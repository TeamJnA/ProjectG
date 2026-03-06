// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Manager/PGLevelGenerator.h"

#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"

#include "Item/PGItemActor.h"
#include "Item/PGItemData.h"

#include "Level/Room/PGMasterRoom.h"
#include "Level/Room/PGStartRoom.h"
#include "Level/Room/PGRoom1.h"
#include "Level/Room/PGRoom2.h"
#include "Level/Room/PGRoom3.h"
#include "Level/Room/PGStairRoom1.h"
#include "Level/Room/PGMirrorRoom.h"
#include "Level/Room/PGElevatorRoom.h"
#include "Level/Room/PGBarrelRoom.h"
#include "Level/Room/PGChargerRoom.h"
#include "Level/Room/PGCorridor_Dark.h"
#include "Level/Room/PGCorridor_Simple.h"
#include "Level/Room/PGLibraryRoom.h"
#include "Level/Room/PGSmallCorridor_Bonfire.h"
#include "Level/Room/PGSmallCorridor_Dark.h"
#include "Level/Room/PGSmallCorridor_Mannequin.h"
#include "Level/Room/PGStairRoom_Simple.h"

#include "Level/Misc/PGDoor1.h"
#include "Level/Misc/PGWall.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickMannequin.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"

// Sets default values
APGLevelGenerator::APGLevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	RoomsList = {
		APGRoom2::StaticClass(),
		APGRoom3::StaticClass(),
		APGStairRoom1::StaticClass()
	};

	// max room spawn amount
	RoomAmount = 26;

	// reload level if (elpased time > max generation time)
	MaxGenerateTime = 10.0f;

	static ConstructorHelpers::FClassFinder<AActor> BlindCharacterRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/BP_BlindCharacter.BP_BlindCharacter_C"));
	if (BlindCharacterRef.Class)
	{
		BlindCharacter = BlindCharacterRef.Class;
	}	
	
	static ConstructorHelpers::FClassFinder<AActor> ChargerCharacterRef(TEXT("/Game/ProjectG/Enemy/Charger/Character/BP_PGChargerCharacter.BP_PGChargerCharacter_C"));
	if (ChargerCharacterRef.Class)
	{
		ChargerCharacter = ChargerCharacterRef.Class;
	}

	static ConstructorHelpers::FClassFinder<AActor> MannequinRef(TEXT("/Game/ProjectG/Gimmick/Trigger/Mannequin/BP_PGTriggerGimmickMannequin.BP_PGTriggerGimmickMannequin_C"));
	if (MannequinRef.Succeeded())
	{
		MannequinClass = MannequinRef.Class;
	}
}

/*
* 레벨 생성 시작
* 레벨 내 생성된 Room들의 ExitPoint(ExitPointsList)에서 새로운 Room 생성 시도
* Overlap 검사 실패 시 새로운 ExitPoint를 골라 재생성
* Overlap 검사 성공 시 ExitPointsList에 새로 생긴 Room의 ExitPoint를 추가하여 새로운 ExitPoint 선택/Room 생성
* 모든 Room 생성 후 Room이 연결되지 않은 모든 ExitPoint에 벽 스폰
* Room이 연결된 ExitPoint들 중 선택하여 문 스폰
* Room의 지정된 ItemSpawnPoint들 중 선택하여 아이템 스폰
* 레벨 생성 과정에서 구축한 RoomGraph를 활용해 StartRoom에서 중간 거리의 Room들을 찾아 선택하여 적대 AI 스폰
*/
void APGLevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetSeed();
		SpawnStartRoom();

		GenerationStartTime = GetWorld()->GetTimeSeconds();
		StartLevelGenerateTimer();

		SpawnNextRoom();
	}
}

/*
* 시드 설정
* 시드값이 없는 경우 랜덤 초기화
* 시드값을 정한 경우 해당 시드값 사용
*/
void APGLevelGenerator::SetSeed()
{
	if (SeedValue == -1)
	{
		Seed.Initialize(FDateTime::Now().GetTicks());
	}
	else
	{
		Seed.Initialize(SeedValue);
	}
}

/*
* StartRoom 스폰
* RoomGraph 첫 노드로 추가
* 기본 아이템, 탈출구 스폰
*/
void APGLevelGenerator::SpawnStartRoom()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform SpawnTransform = Root->GetComponentTransform();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APGStartRoom* NewRoom = World->SpawnActor<APGStartRoom>(APGStartRoom::StaticClass(), SpawnTransform, SpawnParams);
	if (!NewRoom)
	{
		return;
	}
	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
	if (!GI)
	{
		return;
	}

	// Add to RoomGraph
	StartRoom = NewRoom;
	RoomGraph.Add(NewRoom);

	// MasterRoom .h
	// virtual const USceneComponent* GetExitPointsFolder() const { return ExitPointsFolder; }
	if (const USceneComponent* ExitPointsFolder = NewRoom->GetExitPointsFolder())
	{
		const TArray<USceneComponent*>& ExitPoints = ExitPointsFolder->GetAttachChildren();
		ExitPointsList.Reserve(ExitPointsList.Num() + ExitPoints.Num());
		ExitPointsList.Append(ExitPoints);
	}

	APGItemActor* DefaultKey1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData1 = GI->GetItemDataByKey("Key"))
	{
		DefaultKey1->InitWithData(ItemData1);
	}
	DefaultKey1->SetActorRelativeLocation(FVector(538.0f, 271.0f, 90.0f));

	APGItemActor* DeafultKey2 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData2 = GI->GetItemDataByKey("Key"))
	{
		DeafultKey2->InitWithData(ItemData2);
	}
	DeafultKey2->SetActorRelativeLocation(FVector(538.0f, 356.0f, 90.0f));

	
	// for test ~
	APGItemActor* TestChainKey = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData3 = GI->GetItemDataByKey("ChainKey"))
	{
		TestChainKey->InitWithData(ItemData3);
	}
	TestChainKey->SetActorRelativeLocation(FVector(700.0f, 270.0f, 30.0f));

	APGItemActor* TestHandWheel = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData4 = GI->GetItemDataByKey("HandWheel"))
	{
		TestHandWheel->InitWithData(ItemData4);
	}
	TestHandWheel->SetActorRelativeLocation(FVector(700.0f, 350.0f, 30.0f));

	APGItemActor* TestRustOil = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData5 = GI->GetItemDataByKey("RustOil"))
	{
		TestRustOil->InitWithData(ItemData5);
	}
	TestRustOil->SetActorRelativeLocation(FVector(700.0f, 430.0f, 30.0f));

	APGItemActor* TestReviveKit1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData6 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit1->InitWithData(ItemData6);
	}
	TestReviveKit1->SetActorRelativeLocation(FVector(900.0f, 270.0f, 10.0f));

	APGItemActor* TestReviveKit2 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData7 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit2->InitWithData(ItemData7);
	}
	TestReviveKit2->SetActorRelativeLocation(FVector(900.0f, 350.0f, 10.0f));

	APGItemActor* TestReviveKit3 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData8 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit3->InitWithData(ItemData8);
	}
	TestReviveKit3->SetActorRelativeLocation(FVector(900.0f, 430.0f, 10.0f));

	APGItemActor* TestFuse0 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData9 = GI->GetItemDataByKey("Fuse"))
	{
		TestFuse0->InitWithData(ItemData9);
	}
	TestFuse0->SetActorRelativeLocation(FVector(1200.0f, 270.0f, 10.0f));

	APGItemActor* TestFuse1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData10 = GI->GetItemDataByKey("Fuse"))
	{
		TestFuse1->InitWithData(ItemData10);
	}
	TestFuse1->SetActorRelativeLocation(FVector(1200.0f, 350.0f, 10.0f));

	APGItemActor* TestMatch0 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData11 = GI->GetItemDataByKey("Match"))
	{
		TestMatch0->InitWithData(ItemData11);
	}
	TestMatch0->SetActorRelativeLocation(FVector(1400.0f, 270.0f, 10.0f));

	APGItemActor* TestMatch1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData12 = GI->GetItemDataByKey("Match"))
	{
		TestMatch1->InitWithData(ItemData12);
	}
	TestMatch1->SetActorRelativeLocation(FVector(1400.0f, 350.0f, 10.0f));
	// ~ for test

}

/*
* 다음 Room 생성
* RoomAmount > 14(초기단계) -> 복도형 Room만 생성
* RoomAmount <= 14 -> 모든 Room 클래스 중 선택하여 생성
* Room 생성 후 Overlap 검사
*/
void APGLevelGenerator::SpawnNextRoom()
{
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Log, TEXT("LG::SpawnNextRoom: Generation stopped"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World || ExitPointsList.IsEmpty())
	{
		return;
	}

	const TObjectPtr<USceneComponent> SelectedExitPoint = ExitPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, ExitPointsList.Num())];
	const FTransform SpawnTransform(SelectedExitPoint->GetComponentRotation(), SelectedExitPoint->GetComponentLocation());

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APGMasterRoom* NewRoom = nullptr;
	// Corridor 3
	if (RoomAmount > 23)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGRoom1::StaticClass(), SpawnTransform, spawnParams);
	}
	// Coddidor_Dark 2
	else if (RoomAmount > 21)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGCorridor_Dark::StaticClass(), SpawnTransform, spawnParams);
	}
	// Corridor_Simple 1
	else if (RoomAmount > 20)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGCorridor_Simple::StaticClass(), SpawnTransform, spawnParams);
	}
	// BedRoom 2
	else if (RoomAmount > 18)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGRoom3::StaticClass(), SpawnTransform, spawnParams);
	}
	// SmallCorridor_Mannequin 1
	else if (RoomAmount > 17)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGSmallCorridor_Mannequin::StaticClass(), SpawnTransform, spawnParams);
	}
	// Library 1
	else if (RoomAmount > 16)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGLibraryRoom::StaticClass(), SpawnTransform, spawnParams);
	}
	// SmallCorridor 2
	else if (RoomAmount > 14)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGRoom2::StaticClass(), SpawnTransform, spawnParams);
	}
	// Corridor 1
	else if (RoomAmount > 13)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGRoom1::StaticClass(), SpawnTransform, spawnParams);
	}
	// SmallCorridor_Dark 2
	else if (RoomAmount > 11)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGSmallCorridor_Dark::StaticClass(), SpawnTransform, spawnParams);
	}
	// Corridor_Dark 1
	else if (RoomAmount > 10)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGCorridor_Dark::StaticClass(), SpawnTransform, spawnParams);
	}
	// StairRoom_Simple 2
	else if (RoomAmount > 8)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGStairRoom_Simple::StaticClass(), SpawnTransform, spawnParams);
	}
	// Corridor_Simple 1
	else if (RoomAmount > 7)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGCorridor_Simple::StaticClass(), SpawnTransform, spawnParams);
	}
	// StairRoom 1
	else if (RoomAmount > 6)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGStairRoom1::StaticClass(), SpawnTransform, spawnParams);
	}
	// SmallCorridor_Bonfire 1
	else if (RoomAmount > 5)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGSmallCorridor_Bonfire::StaticClass(), SpawnTransform, spawnParams);
	}
	// SmallCorridor_Mannequin 1
	else if (RoomAmount > 4)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGSmallCorridor_Mannequin::StaticClass(), SpawnTransform, spawnParams);
	}
	// BarrelRoom 1
	else if (RoomAmount > 3)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGBarrelRoom::StaticClass(), SpawnTransform, spawnParams);
	}
	// ChargerRoom 1
	else if (RoomAmount > 2)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGChargerRoom::StaticClass(), SpawnTransform, spawnParams);
	}
	// ElevatorRoom 1
	else if (RoomAmount > 1)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGElevatorRoom::StaticClass(), SpawnTransform, spawnParams);
	}
	// MirrorRoom 1
	else
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGMirrorRoom::StaticClass(), SpawnTransform, spawnParams);
	}

	TWeakObjectPtr<APGLevelGenerator> WeakThis(this);
	TWeakObjectPtr<APGMasterRoom> WeakNewRoom(NewRoom);
	TWeakObjectPtr<USceneComponent> WeakSelectedExitPoint(SelectedExitPoint);

	FTimerHandle DelayTimerHandle;
	World->GetTimerManager().SetTimer(
		DelayTimerHandle,
		FTimerDelegate::CreateLambda([WeakThis, WeakSelectedExitPoint, WeakNewRoom]()
		{
			if (WeakThis.IsValid() && WeakSelectedExitPoint.IsValid() && WeakNewRoom.IsValid())
			{
				WeakThis->CheckOverlap(WeakSelectedExitPoint.Get(), WeakNewRoom.Get());
			}
		}), 
		0.1f,
		false
	);
}

/*
* 새로운 Room에 대한 Overlap 검사 및 이후 동작
* Overlap 새로 생성된 Room을 지우고 다시 생성
* Overlap x -> RoomGraph에 추가 -> 부모 Room의 인접 Room으로 추가, 본인의 인접 Room으로 부모 Room 추가
* 사용된 ExitPoint를 ExitPointsList에서 제거, DoorPointsList에 추가
* 생성 확정된 Room의 ExitPoints를 ExitPointsList에 추가,
* 생성 확정된 Room의 ItemSpawnPoints를 ItemSpawnPointsList에 추가
* RoomAmount가 남은 경우 다음 Room 생성
* RoomAmount를 모두 소모한 경우 Room 생성 종료, 구조물 생성
*/
void APGLevelGenerator::CheckOverlap(TObjectPtr<USceneComponent> InSelectedExitPoint, TObjectPtr<APGMasterRoom> RoomToCheck)
{	
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Log, TEXT("LG::CheckOverlap: Generation stopped"));
		if (RoomToCheck)
		{
			RoomToCheck->Destroy();
		}
		return;
	}

	if (IsLatestRoomOverlapping(RoomToCheck))
	{
		RoomToCheck->Destroy();
		SpawnNextRoom();
	}
	else
	{
		// MirrorRoom인 경우 RoomGraph, DoorPointsList 추가 x
		if (RoomAmount > 1)
		{
			// add to room graph
			APGMasterRoom* ParentRoom = Cast<APGMasterRoom>(InSelectedExitPoint->GetOwner());
			if (ParentRoom)
			{
				RoomGraph.FindOrAdd(ParentRoom).Add(RoomToCheck);
				RoomGraph.FindOrAdd(RoomToCheck).Add(ParentRoom);
			}
			DoorPointsList.Add(InSelectedExitPoint);
		}
		ExitPointsList.Remove(InSelectedExitPoint);

		RoomAmount--;

		// MasterRoom .h
		// virtual const USceneComponent* GetExitPointsFolder() const { return ExitPointsFolder; }
		if (const USceneComponent* LatestRoomExitPointsFolder = RoomToCheck->GetExitPointsFolder())
		{
			const TArray<USceneComponent*>& LatestRoomExitPoints = LatestRoomExitPointsFolder->GetAttachChildren();
			ExitPointsList.Reserve(ExitPointsList.Num() + LatestRoomExitPoints.Num());
			ExitPointsList.Append(LatestRoomExitPoints);
		}

		// MasterRoom .h
		// virtual const USceneComponent* GetItemSpawnPointsFolder() const { return ItemSpawnPointsFolder; }
		if (const USceneComponent* ItemSpawnPointFolder = RoomToCheck->GetItemSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& ItemSpawnPoints = ItemSpawnPointFolder->GetAttachChildren();
			ItemSpawnPointsList.Reserve(ItemSpawnPointsList.Num() + ItemSpawnPoints.Num());
			ItemSpawnPointsList.Append(ItemSpawnPoints);
		}

		// MasterRoom.h
		// virtual const USceneComponent* GetMannequinSpawnPointsFolder() const { return MannequinSpawnPointsFolder; }
		if (const USceneComponent* MannequinSpawnPointFolder = RoomToCheck->GetMannequinSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& MannequinSpawnPoints = MannequinSpawnPointFolder->GetAttachChildren();
			MannequinSpawnPointsList.Reserve(MannequinSpawnPointsList.Num() + MannequinSpawnPoints.Num());
			MannequinSpawnPointsList.Append(MannequinSpawnPoints);
		}

		if (RoomAmount > 0)
		{
			SpawnNextRoom();
		}
		else
		{
			SetupLevelEnvironment();
		}
	}
}

/*
* Overlap 검사 구현부
* 생성된 Room의 OverlapBox와 충돌하는 오브젝트 확인
*/
bool APGLevelGenerator::IsLatestRoomOverlapping(const APGMasterRoom* RoomToCheck) const
{
	UWorld* World = GetWorld();
	if (!World || !RoomToCheck)
	{
		return false;
	}

	if (const USceneComponent* OverlapFolder = RoomToCheck->GetOverlapBoxFolder())
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(RoomToCheck);

		for (const USceneComponent* SceneComp : OverlapFolder->GetAttachChildren())
		{
			if (const UBoxComponent* BoxComp = Cast<UBoxComponent>(SceneComp))
			{
				const bool bIsOverlapping = World->OverlapAnyTestByObjectType(
					BoxComp->GetComponentLocation(),
					BoxComp->GetComponentQuat(),
					FCollisionObjectQueryParams(ECollisionChannel::ECC_GameTraceChannel1),
					BoxComp->GetCollisionShape(),
					QueryParams
				);

				if (bIsOverlapping)
				{
					return true;
				}
			}
		}
	}

	return false;
}

/*
* Room 생성 종료 후 구조물 생성
* 벽, 문, 아이템 스폰
* 적대 AI 스폰
* 레벨 생성 작업 완료 후 GameState의 레벨 생성 완료 델리게이트 트리거
* 변수 초기화
*/
void APGLevelGenerator::SetupLevelEnvironment()
{
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SetupLevelEnvironment: Generation stopped by Timeout."));
		return;
	}
	bIsGenerationStopped = true;
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	CloseHoles();
	SpawnDoors();
	SpawnItems();
	SpawnMannequins();
	if (!SpawnEnemy())
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SetupLevelEnvironment: Enemy spawn failed. Restarting Level..."));
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		FTimerHandle TravelStartTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TravelStartTimerHandle,
			this,
			&APGLevelGenerator::ReGenerateLevel,
			1.0f,
			false
		);

		return;
	}
	ComputeExplorationWaypoints();

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->NotifyMapGenerationComplete();
	}

	ExitPointsList.Empty();
	DoorPointsList.Empty();
	MannequinSpawnPointsList.Empty();
	RoomsList.Empty();
	RoomGraph.Empty();
}

/*
* 모든 Room 생성 후 Room과 Room이 연결되지 않은 부분(뚫려있는 부분)에 벽 생성
*/
void APGLevelGenerator::CloseHoles()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (USceneComponent* ExitPoint : ExitPointsList)
	{
		if (!ExitPoint)
		{
			continue;
		}

		const FVector SpawnLocation = ExitPoint->GetComponentLocation();
		FRotator SpawnRotation = ExitPoint->GetComponentRotation();
		SpawnRotation.Yaw += 90.0f;
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGMasterRoom* OwnerRoom = Cast<APGMasterRoom>(ExitPoint->GetOwner());
		if (OwnerRoom && OwnerRoom->GetWallClass())
		{
			TSubclassOf<AActor> WallClass = OwnerRoom->GetWallClass();
			World->SpawnActor<AActor>(WallClass, SpawnTransform, SpawnParams);
		}
		else
		{
			World->SpawnActor<APGWall>(APGWall::StaticClass(), SpawnTransform, SpawnParams);
		}
	}
}

/*
* 모든 Room 생성 후 Room과 Room이 연결된 부분(DoorPointsList)에 문 생성
* 일부는 문이 없거나 잠긴 문 생성
*/
void APGLevelGenerator::SpawnDoors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 DoorAmount = DoorPointsList.Num() * 0.8f;
	int32 LockedDoorAmount = DoorAmount * 0.3f;

	while (DoorAmount > 0 && !DoorPointsList.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, DoorPointsList.Num());
		const TObjectPtr<USceneComponent> SelectedDoorPoint = DoorPointsList[RandomIndex];
		DoorPointsList.RemoveAt(RandomIndex);
		if (!SelectedDoorPoint)
		{
			continue;
		}

		const FVector SpawnLocation = SelectedDoorPoint->GetComponentLocation();
		const FRotator SpawnRotation = SelectedDoorPoint->GetComponentRotation();
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;

		const bool bShouldBeLocked = (LockedDoorAmount > 0);

		check(PGDoor);
		APGDoor1* DefaultDoor = PGDoor->GetDefaultObject<APGDoor1>();
		if (DefaultDoor)
		{
			DefaultDoor->SpawnDoor(World, PGDoor, SpawnTransform, SpawnParams, bShouldBeLocked);
		}

		if (bShouldBeLocked)
		{
			LockedDoorAmount--;
		}
		DoorAmount--;
	}
}

/*
* 모든 Room 생성 후 아이템 스폰
* 재귀를 통한 비동기 아이템 로드/스폰
* 아이템 스폰 완료 후 ItemSpawnPointsList 초기화
*/
void APGLevelGenerator::SpawnItems()
{
	const int32 ItemAmount = 24;
	SpawnSingleItem_Async(ItemAmount);
}

void APGLevelGenerator::SpawnSingleItem_Async(int32 ItemAmount)
{
	UWorld* World = GetWorld();
	if (!World || ItemAmount <= 0 || ItemSpawnPointsList.IsEmpty())
	{
		ItemSpawnPointsList.Empty();
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI)
	{
		return;
	}

	const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, ItemSpawnPointsList.Num());
	const TObjectPtr<USceneComponent> SelectedItemSpawnPoint = ItemSpawnPointsList[RandomIndex];
	ItemSpawnPointsList.RemoveAt(RandomIndex);
	if (!SelectedItemSpawnPoint)
	{
		SpawnSingleItem_Async(ItemAmount);
		return;
	}

	FName ItemKeyToLoad;
	if (ItemAmount > 23)
	{
		ItemKeyToLoad = FName("ChainKey");
	}
	else if (ItemAmount > 22)
	{
		ItemKeyToLoad = FName("HandWheel");
	}
	else if (ItemAmount > 21)
	{
		ItemKeyToLoad = FName("RustOil");
	}
	else if (ItemAmount > 13)
	{
		ItemKeyToLoad = FName("Brick");
	}
	else if (ItemAmount > 5)
	{
		ItemKeyToLoad = FName("Key");
	}
	else
	{
		ItemKeyToLoad = FName("ReviveKit");
	}

	ItemAmount--;

	const FTransform SpawnTransform(FRotator::ZeroRotator, SelectedItemSpawnPoint->GetComponentLocation());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GI->RequestLoadItemData(ItemKeyToLoad, FOnItemDataLoaded::CreateLambda([World, SpawnTransform, SpawnParams, ItemAmount, this](UPGItemData* LoadedItemData)
	{
		if (LoadedItemData)
		{
			APGItemActor* NewItem = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
			if (NewItem)
			{
				NewItem->InitWithData(LoadedItemData);
			}
		}

		SpawnSingleItem_Async(ItemAmount);
	}));
}

void APGLevelGenerator::SpawnMannequins()
{
	UWorld* World = GetWorld();
	if (!World || MannequinSpawnPointsList.IsEmpty())
	{
		return;
	}

	int32 MannequinAmount = MannequinSpawnPointsList.Num() * 0.4f;

	while (MannequinAmount > 0 && !MannequinSpawnPointsList.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, MannequinSpawnPointsList.Num());
		const TObjectPtr<USceneComponent> SelectedSpawnPoint = MannequinSpawnPointsList[RandomIndex];
		MannequinSpawnPointsList.RemoveAt(RandomIndex);

		if (SelectedSpawnPoint)
		{
			const FTransform SpawnTransform(SelectedSpawnPoint->GetComponentTransform());
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			World->SpawnActor<APGTriggerGimmickMannequin>(MannequinClass, SpawnTransform, SpawnParams);
			MannequinAmount--;
		}
	}
}

/*
* 모든 Room 생성 후 적대 AI 스폰
* StartRoom 기준 중간 거리의 Room에 적대 AI 스폰
*/
bool APGLevelGenerator::SpawnEnemy()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APGGameMode* GM = World->GetAuthGameMode<APGGameMode>();
	if (!GM)
	{
		return false;
	}

	//APGMasterRoom* EnemySpawnRoom = FindFarthestRoom();
	const APGMasterRoom* BlindSpawnRoom = FindMiddleDistanceRoom();
	if (BlindSpawnRoom)
	{
		const FTransform SpawnTransform(FRotator::ZeroRotator, BlindSpawnRoom->GetEnemySpawnLocation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGBlindCharacter* SpawnedBlindCharacter = World->SpawnActor<APGBlindCharacter>(BlindCharacter, SpawnTransform, SpawnParams);
		if (SpawnedBlindCharacter)
		{
			SpawnedBlindCharacter->InitSoundManager(GM->GetSoundManager());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn blind"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("LG::SpawnEnemy: Spawn enemy at room '%s'"), *BlindSpawnRoom->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn blind"));
		return false;
	}

	const APGMasterRoom* ChargerSpawnRoom = FindMiddleDistanceRoom(BlindSpawnRoom->GetEnemySpawnLocation());
	if (ChargerSpawnRoom)
	{
		const FTransform ChargerSpawnTransform(FRotator::ZeroRotator, ChargerSpawnRoom->GetEnemySpawnLocation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGChargerCharacter* SpawnedCharger = World->SpawnActor<APGChargerCharacter>(ChargerCharacter, ChargerSpawnTransform, SpawnParams);
		if (SpawnedCharger)
		{
			SpawnedCharger->InitSoundManager(GM->GetSoundManager());
			UE_LOG(LogTemp, Log, TEXT("LG::SpawnEnemy: Spawn Charger at room '%s'"), *ChargerSpawnRoom->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn charger"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn charger"));
		return false;
	}

	const APGMasterRoom* GhostSpawnRoom = FindFarthestRoom();
	if (GhostSpawnRoom)
	{
		const FTransform SpawnTransform(FRotator::ZeroRotator, GhostSpawnRoom->GetEnemySpawnLocation());
		UE_LOG(LogTemp, Log, TEXT("LG::SpawnEnemy: Spawn ghost. (Room: %s, Location: %s)"), *GhostSpawnRoom->GetName(), *SpawnTransform.GetLocation().ToString());
		GM->SpawnGhost(SpawnTransform);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn ghost"));
		return false;
	}

	return true;
}

/*
* ExitPoint가 부족하여 더이상 Room을 생성할 수 없는 상황을 방지하기 위해 타이머 체크
* MaxGenerateTime(8초) 이후에도 레벨 생성 중이라면 재생성
*/
void APGLevelGenerator::StartLevelGenerateTimer()
{
	FTimerHandle LevelGenerateTimer;
	GetWorld()->GetTimerManager().SetTimer(
		LevelGenerateTimer,
		this,
		&APGLevelGenerator::CheckLevelGenerateTimeOut,
		1.0f,
		true,
		0.0f
	);
}

/*
* elapsed time 체크
*/
void APGLevelGenerator::CheckLevelGenerateTimeOut()
{
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::CheckLevelGenerateTimeOut: Level generation already successed"));
		return;
	}

	const float ElapsedTime = GetWorld()->GetTimeSeconds() - GenerationStartTime;
	if (ElapsedTime >= MaxGenerateTime)
	{
		bIsGenerationStopped = true;
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		UE_LOG(LogTemp, Log, TEXT("LG::CheckLevelGenerateTimeout: Timeout. re-open level"));
		FTimerHandle TravelStartTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TravelStartTimerHandle,
			this,
			&APGLevelGenerator::ReGenerateLevel,
			1.0f,
			false
		);
	}
}

void APGLevelGenerator::ReGenerateLevel()
{
	UE_LOG(LogTemp, Log, TEXT("LG::ReGenerateLevel: ServerTravel."));
	GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
}

/*
* RoomGrpah에 대해 BFS 방식으로 StartRoom에서 가장 먼 Room 탐색
* 가장 먼 Room 중 랜덤 석택
*/
const APGMasterRoom* APGLevelGenerator::FindFarthestRoom() const
{
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		return nullptr;
	}

	TQueue<TObjectPtr<APGMasterRoom>> RoomsToVisit;
	TMap<TObjectPtr<APGMasterRoom>, int32> Distances;

	RoomsToVisit.Enqueue(StartRoom);
	Distances.Add(StartRoom, 0);

	// BFS
	while (!RoomsToVisit.IsEmpty())
	{
		TObjectPtr<APGMasterRoom> CurrentRoom;
		RoomsToVisit.Dequeue(CurrentRoom);

		const int32 CurrentDistance = Distances[CurrentRoom];

		if (const TArray<TObjectPtr<APGMasterRoom>>* Neighbors = RoomGraph.Find(CurrentRoom))
		{
			for (const TObjectPtr<APGMasterRoom> Neighbor : *Neighbors)
			{
				if (Neighbor && !Distances.Contains(Neighbor))
				{
					Distances.Add(Neighbor, CurrentDistance + 1);
					RoomsToVisit.Enqueue(Neighbor);
				}
			}
		}
	}

	// find farthest room	
	TArray<TObjectPtr<APGMasterRoom>> FarthestRooms;
	int32 MaxDistance = -1;
	for (const auto& Elem : Distances)
	{
		if (Elem.Value > MaxDistance)
		{
			MaxDistance = Elem.Value;

			FarthestRooms.Empty();
			FarthestRooms.Add(Elem.Key);
		}
		else if (Elem.Value == MaxDistance)
		{
			FarthestRooms.Add(Elem.Key);
		}
	}

	// random select
	if (!FarthestRooms.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, FarthestRooms.Num());
		return FarthestRooms[RandomIndex];
	}	
		
	return nullptr;
}

/*
* RoomGrpah에 대해 BFS 방식으로 StartRoom에서 중간 거리 Room 탐색
* 중간 거리 Room 중 랜덤 선택
*/
const APGMasterRoom* APGLevelGenerator::FindMiddleDistanceRoom(const FVector& AvoidLocation) const
{
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		return nullptr;
	}

	TQueue<TObjectPtr<APGMasterRoom>> RoomsToVisit;
	TMap<TObjectPtr<APGMasterRoom>, int32> Distances;

	RoomsToVisit.Enqueue(StartRoom);
	Distances.Add(StartRoom, 0);
	int32 MaxDistance = 0;

	// BFS
	while (!RoomsToVisit.IsEmpty())
	{
		TObjectPtr<APGMasterRoom> CurrentRoom;
		RoomsToVisit.Dequeue(CurrentRoom);
		const int32 CurrentDistance = Distances[CurrentRoom];
		MaxDistance = FMath::Max(MaxDistance, CurrentDistance);

		if (const TArray<TObjectPtr<APGMasterRoom>>* Neighbors = RoomGraph.Find(CurrentRoom))
		{
			for (const TObjectPtr<APGMasterRoom> Neighbor : *Neighbors)
			{
				if (Neighbor && !Distances.Contains(Neighbor))
				{
					Distances.Add(Neighbor, CurrentDistance + 1);
					RoomsToVisit.Enqueue(Neighbor);
				}
			}
		}
	}

	// find middle distance room
	const int32 TargetDistance = MaxDistance / 2 + 1;
	TArray<TObjectPtr<APGMasterRoom>> MiddleDistanceRooms;
	for (const auto& Elem : Distances)
	{
		if (Elem.Value == TargetDistance)
		{
			MiddleDistanceRooms.Add(Elem.Key);
		}
	}

	if (MiddleDistanceRooms.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[LG::FindMiddleDistanceRoom] no middle distance rooms"));
		return nullptr;
	}

	// if AvoidLocation != 0
	if (!AvoidLocation.IsZero())
	{
		TObjectPtr<APGMasterRoom> FarthestRoom = nullptr;
		float MaxDist = -1.0f;

		for (const TObjectPtr<APGMasterRoom>& Room : MiddleDistanceRooms)
		{
			float Dist = FVector::Dist(Room->GetEnemySpawnLocation(), AvoidLocation);
			if (Dist > MaxDist)
			{
				MaxDist = Dist;
				FarthestRoom = Room;
			}
		}
		return FarthestRoom;
	}

	// Default(AvoidLocation == 0): random select
	const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, MiddleDistanceRooms.Num());
	return MiddleDistanceRooms[RandomIndex];
}

void APGLevelGenerator::ComputeExplorationWaypoints()
{
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[LG] No valid StartRoom, RoomGraph"));
		return;
	}

	TQueue<TObjectPtr<APGMasterRoom>> RoomsToVisit;
	TMap<TObjectPtr<APGMasterRoom>, int32>  Distances;
	TMap<TObjectPtr<APGMasterRoom>, TObjectPtr<APGMasterRoom>> ParentMap;

	RoomsToVisit.Enqueue(StartRoom);
	Distances.Add(StartRoom, 0);

	int32 MaxDistance = 0;

	while (!RoomsToVisit.IsEmpty())
	{
		TObjectPtr<APGMasterRoom> CurrentRoom;
		RoomsToVisit.Dequeue(CurrentRoom);
		const int32 CurrentDistance = Distances[CurrentRoom];
		MaxDistance = FMath::Max(MaxDistance, CurrentDistance);

		if (const TArray<TObjectPtr<APGMasterRoom>>* Neighbors = RoomGraph.Find(CurrentRoom))
		{
			for (const TObjectPtr<APGMasterRoom>& Neighbor : *Neighbors)
			{
				if (Neighbor && !Distances.Contains(Neighbor))
				{
					Distances.Add(Neighbor, CurrentDistance + 1);
					ParentMap.Add(Neighbor, CurrentRoom);
					RoomsToVisit.Enqueue(Neighbor);
				}
			}
		}
	}

	TArray<TObjectPtr<APGMasterRoom>> LeafRooms;
	for (const auto& Elem : RoomGraph)
	{
		if (Elem.Key == StartRoom)
		{
			continue;
		}

		if (Elem.Value.Num() <= 1)
		{
			LeafRooms.Add(Elem.Key);
		}
	}

	TArray<FVector> Waypoints;
	TSet<TObjectPtr<APGMasterRoom>> UsedRooms;

	for (const TObjectPtr<APGMasterRoom>& Leaf : LeafRooms)
	{
		int32 LeafDist = Distances.Contains(Leaf) ? Distances[Leaf] : 0;
		int32 TargetDist = FMath::RoundToInt(LeafDist * 0.75f);

		TObjectPtr<APGMasterRoom> Current = Leaf;
		while (Current && Distances.Contains(Current))
		{
			if (Distances[Current] == TargetDist)
			{
				if (!UsedRooms.Contains(Current))
				{
					UsedRooms.Add(Current);
					Waypoints.Add(Current->GetEnemySpawnLocation());
				}
				break;
			}

			if (ParentMap.Contains(Current))
			{
				Current = ParentMap[Current];
			}
			else
			{
				break;
			}
		}
	}

	if (Waypoints.Num() < 4)
	{
		int32 MidDist = MaxDistance / 2;
		for (const auto& Elem : Distances)
		{
			if (Elem.Key == StartRoom)
			{
				continue;
			}

			if (Elem.Value == MidDist && !UsedRooms.Contains(Elem.Key))
			{
				Waypoints.Add(Elem.Key->GetEnemySpawnLocation());
			}
		}
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->SetExplorationWaypoints(Waypoints);
		UE_LOG(LogTemp, Log, TEXT("LG::ComputeExplorationWaypoints: Generated %d waypoints"), Waypoints.Num());

#if !UE_BUILD_SHIPPING
		GS->DrawDebugWaypoints();
#endif
	}
}
