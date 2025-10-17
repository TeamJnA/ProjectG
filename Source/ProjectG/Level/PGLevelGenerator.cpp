// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLevelGenerator.h"

#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"

#include "Item/PGItemActor.h"
#include "Item/PGItemData.h"

#include "PGMasterRoom.h"
#include "PGStartRoom.h"
#include "PGRoom1.h"
#include "PGRoom2.h"
#include "PGRoom3.h"
#include "PGStairRoom1.h"
#include "PGDoor1.h"
#include "PGWall.h"
#include "PGExitDoor.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"

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
	RoomAmount = 20;

	// reload level if (elpased time > max generation time)
	MaxGenerateTime = 8.0f;

	static ConstructorHelpers::FClassFinder<AActor> BlindCharacterRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/BP_BlindCharacter.BP_BlindCharacter_C"));
	if (BlindCharacterRef.Class)
	{
		BlindCharacter = BlindCharacterRef.Class;
	}
}

/*
* ���� ���� ����
* ���� �� ������ Room���� ExitPoint(ExitPointsList)���� ���ο� Room ���� �õ�
* Overlap �˻� ���� �� ���ο� ExitPoint�� ��� �����
* Overlap �˻� ���� �� ExitPointsList�� ���� ���� Room�� ExitPoint�� �߰��Ͽ� ���ο� ExitPoint ����/Room ����
* ��� Room ���� �� Room�� ������� ���� ��� ExitPoint�� �� ����
* Room�� ����� ExitPoint�� �� �����Ͽ� �� ����
* Room�� ������ ItemSpawnPoint�� �� �����Ͽ� ������ ����
* ���� ���� �������� ������ RoomGraph�� Ȱ���� StartRoom���� �߰� �Ÿ��� Room���� ã�� �����Ͽ� ���� AI ����
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
* �õ� ����
* �õ尪�� ���� ��� ���� �ʱ�ȭ
* �õ尪�� ���� ��� �ش� �õ尪 ���
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
* StartRoom ����
* RoomGraph ù ���� �߰�
* �⺻ ������, Ż�ⱸ ����
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
	APGItemActor* TestExitKey1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData3 = GI->GetItemDataByKey("ExitKey"))
	{
		TestExitKey1->InitWithData(ItemData3);
	}
	TestExitKey1->SetActorRelativeLocation(FVector(700.0f, 270.0f, 10.0f));

	APGItemActor* TestExitKey2 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData4 = GI->GetItemDataByKey("ExitKey"))
	{
		TestExitKey2->InitWithData(ItemData4);
	}
	TestExitKey2->SetActorRelativeLocation(FVector(700.0f, 350.0f, 10.0f));

	APGItemActor* TestExitKey3 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ItemData5 = GI->GetItemDataByKey("ExitKey"))
	{
		TestExitKey3->InitWithData(ItemData5);
	}
	TestExitKey3->SetActorRelativeLocation(FVector(700.0f, 430.0f, 10.0f));
	// ~ for test


	APGExitDoor* ExitDoor = World->SpawnActor<APGExitDoor>(APGExitDoor::StaticClass(), SpawnParams);
	ExitDoor->SetActorRelativeLocation(FVector(1850.0f, 317.0f, 10.0f));
	ExitDoor->SetActorRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

/*
* ���� Room ����
* RoomAmount > 14(�ʱ�ܰ�) -> ������ Room�� ����
* RoomAmount <= 14 -> ��� Room Ŭ���� �� �����Ͽ� ����
* Room ���� �� Overlap �˻�
*/
void APGLevelGenerator::SpawnNextRoom()
{
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
	if (RoomAmount > 14)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGRoom1::StaticClass(), SpawnTransform, spawnParams);
	}
	else
	{
		const TSubclassOf<APGMasterRoom>& NewRoomClass = RoomsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, RoomsList.Num())];
		NewRoom = World->SpawnActor<APGMasterRoom>(NewRoomClass, SpawnTransform, spawnParams);
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
* ���ο� Room�� ���� Overlap �˻� �� ���� ����
* Overlap ���� ������ Room�� ����� �ٽ� ����
* Overlap x -> RoomGraph�� �߰� -> �θ� Room�� ���� Room���� �߰�, ������ ���� Room���� �θ� Room �߰�
* ���� ExitPoint�� ExitPointsList���� ����, DoorPointsList�� �߰�
* ���� Ȯ���� Room�� ExitPoints�� ExitPointsList�� �߰�,
* ���� Ȯ���� Room�� ItemSpawnPoints�� ItemSpawnPointsList�� �߰�
* RoomAmount�� ���� ��� ���� Room ����
* RoomAmount�� ��� �Ҹ��� ��� Room ���� ����, ������ ����
*/
void APGLevelGenerator::CheckOverlap(TObjectPtr<USceneComponent> InSelectedExitPoint, TObjectPtr<APGMasterRoom> RoomToCheck)
{	
	if (IsLatestRoomOverlapping(RoomToCheck))
	{
		RoomToCheck->Destroy();
		SpawnNextRoom();
	}
	else
	{
		// add to room graph
		APGMasterRoom* ParentRoom = Cast<APGMasterRoom>(InSelectedExitPoint->GetOwner());
		if (ParentRoom)
		{
			RoomGraph.FindOrAdd(ParentRoom).Add(RoomToCheck);
			RoomGraph.FindOrAdd(RoomToCheck).Add(ParentRoom);
		}

		ExitPointsList.Remove(InSelectedExitPoint);
		DoorPointsList.Add(InSelectedExitPoint);

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
* Overlap �˻� ������
* ������ Room�� OverlapBox�� �浹�ϴ� ������Ʈ Ȯ��
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
* Room ���� ���� �� ������ ����
* ��, ��, ������ ����
* ���� AI ����
* ���� ���� �۾� �Ϸ� �� GameState�� ���� ���� �Ϸ� ��������Ʈ Ʈ����
* ���� �ʱ�ȭ
*/
void APGLevelGenerator::SetupLevelEnvironment()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	CloseHoles();
	SpawnDoors();
	SpawnItems();
	SpawnEnemy();

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->NotifyMapGenerationComplete();
	}

	ExitPointsList.Empty();
	DoorPointsList.Empty();
	RoomsList.Empty();
	RoomGraph.Empty();
}

/*
* ��� Room ���� �� Room�� Room�� ������� ���� �κ�(�շ��ִ� �κ�)�� �� ����
*/
void APGLevelGenerator::CloseHoles()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const TObjectPtr<USceneComponent> ExitPoint : ExitPointsList)
	{
		const FVector SpawnLocation = ExitPoint->GetComponentLocation();
		FRotator SpawnRotation = ExitPoint->GetComponentRotation();
		SpawnRotation.Yaw += 90.0f;
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		World->SpawnActor<APGWall>(APGWall::StaticClass(), SpawnTransform, SpawnParams);
	}
}

/*
* ��� Room ���� �� Room�� Room�� ����� �κ�(DoorPointsList)�� �� ����
* �Ϻδ� ���� ���ų� ��� �� ����
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

		APGDoor1::SpawnDoor(World, SpawnTransform, SpawnParams, bShouldBeLocked);

		if (bShouldBeLocked)
		{
			LockedDoorAmount--;
		}
		DoorAmount--;
	}
}

/*
* ��� Room ���� �� ������ ����
* ��͸� ���� �񵿱� ������ �ε�/����
* ������ ���� �Ϸ� �� ItemSpawnPointsList �ʱ�ȭ
*/
void APGLevelGenerator::SpawnItems()
{
	const int32 ItemAmount = 20;
	SpawnSingleItem_Async(ItemAmount);
}

void APGLevelGenerator::SpawnSingleItem_Async(int32 ItemAmount)
{
	UWorld* World = GetWorld();
	if (!World || ItemAmount <= 0 || ItemSpawnPointsList.IsEmpty())
	{
		return;
		ItemSpawnPointsList.Empty();
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
	if (ItemAmount >= 18)
	{
		ItemKeyToLoad = FName("ExitKey");
	}
	else if (ItemAmount >= 10)
	{
		ItemKeyToLoad = FName("Brick");
	}
	else
	{
		ItemKeyToLoad = FName("Key");
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

/*
* ��� Room ���� �� ���� AI ����
* StartRoom ���� �߰� �Ÿ��� Room�� ���� AI ����
*/
void APGLevelGenerator::SpawnEnemy()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APGGameMode* GM = World->GetAuthGameMode<APGGameMode>();
	if (!GM)
	{
		return;
	}

	//APGMasterRoom* EnemySpawnRoom = FindFarthestRoom();
	const APGMasterRoom* EnemySpawnRoom = FindMiddleDistanceRoom();
	if (EnemySpawnRoom)
	{
		const FTransform SpawnTransform(FRotator::ZeroRotator, EnemySpawnRoom->GetEnemySpawnLocation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGBlindCharacter* SpawnedBlindCharacter = World->SpawnActor<APGBlindCharacter>(BlindCharacter, SpawnTransform, SpawnParams);
		if (SpawnedBlindCharacter)
		{
			SpawnedBlindCharacter->InitSoundManager(GM->GetSoundManager());
		}

		UE_LOG(LogTemp, Log, TEXT("LG::SetupLevelEnvironment: Spawn enemy at room '%s'"), *EnemySpawnRoom->GetName());
	}
}

/*
* ExitPoint�� �����Ͽ� ���̻� Room�� ������ �� ���� ��Ȳ�� �����ϱ� ���� Ÿ�̸� üũ
* MaxGenerateTime(8��) ���Ŀ��� ���� ���� ���̶�� �����
*/
void APGLevelGenerator::StartLevelGenerateTimer() const
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
* elapsed time üũ
*/
void APGLevelGenerator::CheckLevelGenerateTimeOut() const
{
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - GenerationStartTime;
	if (ElapsedTime >= MaxGenerateTime)
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		UE_LOG(LogTemp, Log, TEXT("LG::CheckLevelGenerateTimeout: Timeout. re-open level"));
		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}
}

/*
* RoomGrpah�� ���� BFS ������� StartRoom���� ���� �� Room Ž��
* ���� �� Room �� ���� ����
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
* RoomGrpah�� ���� BFS ������� StartRoom���� �߰� �Ÿ� Room Ž��
* �߰� �Ÿ� Room �� ���� ����
*/
const APGMasterRoom* APGLevelGenerator::FindMiddleDistanceRoom() const
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
	const int32 TargetDistance = MaxDistance / 2;
	TArray<TObjectPtr<APGMasterRoom>> MiddleDistanceRooms;
	for (const auto& Elem : Distances)
	{
		if (Elem.Value == TargetDistance)
		{
			MiddleDistanceRooms.Add(Elem.Key);
		}
	}

	// random select
	if (!MiddleDistanceRooms.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, MiddleDistanceRooms.Num());
		return MiddleDistanceRooms[RandomIndex];
	}

	return nullptr;
}
