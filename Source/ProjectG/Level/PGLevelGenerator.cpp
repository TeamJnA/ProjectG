// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLevelGenerator.h"

#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

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

#include "PGGlobalLightManager.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerController.h"

#include "Net/UnrealNetwork.h"

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
	// max door spawn amount
	// door: created between rooms
	// wall: created between room and outside
	DoorAmount = 0;
	LockedDoorAmount = 0;

	// max item spawn amount
	ItemAmount = 20;

	// reload level if (elpased time > max generation time)
	MaxGenerateTime = 8.0f;

	// TEST TO REMOVE
	BlindCharacter;
	static ConstructorHelpers::FClassFinder<AActor> BlindCharacterRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/BP_BlindCharacter.BP_BlindCharacter_C"));
	if (BlindCharacterRef.Class)
	{
		BlindCharacter = BlindCharacterRef.Class;
	}
}

// Called when the game starts or when spawned
// Spawn everything at only server
void APGLevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetSeed();
		SpawnStartRoom();

		GenerationStartTime = GetWorld()->GetTimeSeconds();
		StartDungeonTimer();

		SpawnNextRoom();
	}
}

// set level seed
// All actors are spawned based on seeds
// If seed value == -1, generate random seed
// If a specific value is specified, set that value as seed.
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

// Create the base room, PGStartRoom
void APGLevelGenerator::SpawnStartRoom()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// spawn setting
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
	// Add the Points(exitpoints) where the next Room can be created to the ExitsList.
	TArray<USceneComponent*> ExitPoints;
	NewRoom->GetExitsFolder()->GetChildrenComponents(false, ExitPoints);
	ExitsList.Append(ExitPoints);

	// spawn default items & exit door
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

	APGExitDoor* ExitDoor = World->SpawnActor<APGExitDoor>(APGExitDoor::StaticClass(), SpawnParams);
	ExitDoor->SetActorRelativeLocation(FVector(1850.0f, 317.0f, 10.0f));
	ExitDoor->SetActorRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

// Create Rooms repeatedly a fixed number of times(RoomAmount)
void APGLevelGenerator::SpawnNextRoom()
{
	UWorld* World = GetWorld();
	if (!World || ExitsList.IsEmpty())
	{
		return;
	}

	// get random room spawn point from ExitsList by stream(seed)
	const TObjectPtr<USceneComponent> SelectedExitPoint = ExitsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, ExitsList.Num())];
	// spawn setting
	const FVector spawnLocation = SelectedExitPoint->GetComponentLocation();
	const FRotator spawnRotation = SelectedExitPoint->GetComponentRotation();
	const FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APGMasterRoom* NewRoom = nullptr;
	if (RoomAmount > 14)
	{
		NewRoom = World->SpawnActor<APGMasterRoom>(APGRoom1::StaticClass(), spawnTransform, spawnParams);
	}
	else
	{
		const TSubclassOf<APGMasterRoom> NewRoomClass = RoomsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, RoomsList.Num())];
		NewRoom = World->SpawnActor<APGMasterRoom>(NewRoomClass, spawnTransform, spawnParams);
	}
	// set spawned room as LatestRoom
	LatestRoom = NewRoom;

	// Check if the created room does not overlap with another room
	// Check if the created room exceeds the room amount
	// Delay before spawn next room
	FTimerHandle TempTimerHandle;
	World->GetTimerManager().SetTimer(
		TempTimerHandle,
		FTimerDelegate::CreateUObject(this, &APGLevelGenerator::CheckOverlap, SelectedExitPoint),
		0.1f,
		false
	);
}

// 1) Check if there is another room overlapping the OverlapBox of LatestRoom(most recently spawned room)
// 2) Check can generate next room
void APGLevelGenerator::CheckOverlap(TObjectPtr<USceneComponent> InSelectedExitPoint)
{
	AddOverlappingRoomsToList();
	
	// 1) Check if there is another room overlapping the OverlapBox of LatestRoom(most recently spawned room)
	if (!OverlappedList.IsEmpty())
	{
		// empty OverlappedList for next spawn room
		// destroy LatestRoom and spawn new room
		OverlappedList.Empty();
		LatestRoom->Destroy();
		SpawnNextRoom();
	}
	// if there is no overlapped room (= success to create new room)
	else
	{
		ExitsList.Remove(InSelectedExitPoint);
		DoorPointsList.Add(InSelectedExitPoint);

		// add new room to graph
		// find connected room(prior or parent room) and add to graph
		APGMasterRoom* ParentRoom = Cast<APGMasterRoom>(InSelectedExitPoint->GetOwner());
		if (ParentRoom)
		{
			// add new room to neighbors list of parent room
			RoomGraph.FindOrAdd(ParentRoom).Add(LatestRoom);
			// add parent room to neighbors list of new room
			RoomGraph.FindOrAdd(LatestRoom).Add(ParentRoom);
		}
		else
		{
			// if there are no parent rooms(should be none other than the starting room), just add a node to the graph. (can not happen)
			RoomGraph.Add(LatestRoom);
		}

		// update RoomAmount
		RoomAmount--;

		// add LatestRoom's exitpoints(where the next Room can be spawned) to the ExitsList
		TArray<USceneComponent*> LatestRoomExitPoints;
		LatestRoom->GetExitsFolder()->GetChildrenComponents(false, LatestRoomExitPoints);
		ExitsList.Append(LatestRoomExitPoints);
		AddFloorSpawnPointsToList();

		// 2) Check can generate next room
		// can generate next room
		if (RoomAmount > 0)
		{
			SpawnNextRoom();
		}
		// Generate Rooms Completely
		// stop generation => spawn walls, doors, items and clear timer, spawn global light manager(PGGlobalLightManager)
		else
		{
			SetupLevelEnvironment();
		}
	}
}

void APGLevelGenerator::SetupLevelEnvironment()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	UE_LOG(LogTemp, Warning, TEXT("LG::SetupLevelEnvironment: Generate Rooms Completely"));

	CloseHoles();
	SpawnDoors();
	SpawnItems();

	// spawn enemy
	//APGMasterRoom* EnemySpawnRoom = FindFarthestRoom();
	APGMasterRoom* EnemySpawnRoom = FindMiddleDistanceRoom();
	if (EnemySpawnRoom)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			APGGameMode* GM = Cast<APGGameMode>(World->GetAuthGameMode());
			if (!GM)
			{
				UE_LOG(LogTemp, Error, TEXT("LG::SetupLevelEnvironment: No valid GM"));
				return;
			}

			// spawn enemy at center of fartherst room
			const FVector SpawnLocation = EnemySpawnRoom->GetEnemySpawnLocation();
			const FRotator SpawnRotation = FRotator::ZeroRotator;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APGBlindCharacter* SpawnedBlindCharacter = World->SpawnActor<APGBlindCharacter>(BlindCharacter, SpawnLocation, SpawnRotation, SpawnParams);
			if (SpawnedBlindCharacter)
			{
				SpawnedBlindCharacter->InitSoundManager(GM->GetSoundManager());
			}

			UE_LOG(LogTemp, Log, TEXT("LG::SetupLevelEnvironment: Spawn enemy at room '%s'"), *EnemySpawnRoom->GetName());
		}
	}

	// trigger a GS' delegate to notify GameMode to spawn the player.
	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SetupLevelEnvironment: Notify map generation complete to GameState"));
		GS->NotifyMapGenerationComplete();
	}

	// 모든 생성 작업이 완료되었으므로, 더 이상 필요 없는 임시 목록들 정리
	UE_LOG(LogTemp, Log, TEXT("LG::SetupLevelEnvironment: Clearing temporary generation data to free memory."));
	ExitsList.Empty();
	DoorPointsList.Empty();
	FloorSpawnPointsList.Empty();
	OverlappedList.Empty();
	RoomsList.Empty();
	RoomGraph.Empty();
	LatestRoom = nullptr;
}

// Check if there is another room overlapping the OverlapBox of LatestRoom(most recently spawned room)
void APGLevelGenerator::AddOverlappingRoomsToList()
{
	OverlappedList.Empty();
	if (!LatestRoom)
	{
		return;
	}

	TArray<USceneComponent*> OverlapItems;
	LatestRoom->GetOverlapBoxFolder()->GetChildrenComponents(false, OverlapItems);
	for (USceneComponent* SceneComp : OverlapItems)
	{
		if (UBoxComponent* BoxComp = Cast<UBoxComponent>(SceneComp))
		{
			TArray<UPrimitiveComponent*> TempOverlaps;
			BoxComp->GetOverlappingComponents(TempOverlaps);
			OverlappedList.Append(TempOverlaps);
		}
	}
}

// Add item spawn points of LatestRoom(most recently spawned room) to FloorSpawnPointsList
void APGLevelGenerator::AddFloorSpawnPointsToList()
{
	if (!LatestRoom)
	{
		return;
	}

	TArray<USceneComponent*> FloorSpawnPoints;
	LatestRoom->GetFloorSpawnPointsFolder()->GetChildrenComponents(false, FloorSpawnPoints);
	FloorSpawnPointsList.Append(FloorSpawnPoints);
}

// spawn walls(created between room and outside)
void APGLevelGenerator::CloseHoles()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// get all exit point from ExitsList then spawn wall actor
	for (const auto& ExitPoint : ExitsList)
	{
		// spawn setting
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

// spawn doors(created between rooms) by a certain number(DoorAmount)
void APGLevelGenerator::SpawnDoors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	DoorAmount = DoorPointsList.Num() * 0.8f;
	LockedDoorAmount = DoorAmount * 0.3f;

	while (DoorAmount > 0 && !DoorPointsList.IsEmpty())
	{
		// get random door spawn point from DoorPointsList by stream(seed)
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, DoorPointsList.Num());
		const TObjectPtr<USceneComponent> SelectedDoorPoint = DoorPointsList[RandomIndex];
		if (!SelectedDoorPoint)
		{
			continue;
		}
		DoorPointsList.RemoveAt(RandomIndex);

		// spawn setting
		const FVector SpawnLocation = SelectedDoorPoint->GetComponentLocation();
		const FRotator SpawnRotation = SelectedDoorPoint->GetComponentRotation();
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;

		const bool bShouldBeLocked = (LockedDoorAmount > 0);
		APGDoor1::SpawnDoor(World, SpawnTransform, SpawnParams, bShouldBeLocked);

		// after spawn, remove used door point from door points list
		if (bShouldBeLocked)
		{
			LockedDoorAmount--;
		}
		DoorAmount--;
	}
}

// spawn items by a certain number(ItemAmount)
void APGLevelGenerator::SpawnItems()
{
	SpawnSingleItem_Async();
}

void APGLevelGenerator::SpawnSingleItem_Async()
{
	UWorld* World = GetWorld();
	// 아이템을 모두 스폰했거나, 스폰 위치가 더이상 없거나, world가 없으면 중단
	if (!World || ItemAmount <= 0 || FloorSpawnPointsList.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("LG::SpawnSingleItem_Async: Item spawning process finished or stopped"));
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("LG::SpawnSingleItem_Async: GI is not valid"));
		return;
	}

	// 1) 스폰 위치 선정 (Seed 기반)
	const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, FloorSpawnPointsList.Num());
	const TObjectPtr<USceneComponent> SelectedFloorSpawnPoint = FloorSpawnPointsList[RandomIndex];
	if (!SelectedFloorSpawnPoint)
	{
		SpawnSingleItem_Async(); // Try next one
		return;
	}
	FloorSpawnPointsList.RemoveAt(RandomIndex);

	// 2) 스폰 아이템 키(key) 결정
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

	// 3) 스폰할 아이템 수 감소
	ItemAmount--;

	// 4) 스폰 위치, 파라미터 준비
	const FVector SpawnLocation = SelectedFloorSpawnPoint->GetComponentLocation();
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 5) GameInstance에 데이터 로드 요청
	// 로드 완료시 람다함수(아이템 스폰) 실행
	GI->RequestLoadItemData(ItemKeyToLoad, FOnItemDataLoaded::CreateLambda([World, SpawnTransform, SpawnParams, this](UPGItemData* LoadedItemData)
	{
		if (World && LoadedItemData)
		{
			APGItemActor* NewItem = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
			if (NewItem)
			{
				NewItem->InitWithData(LoadedItemData);
				UE_LOG(LogTemp, Log, TEXT("LG::SpawnSingleItem_Async: Item Spawned: %s"), *LoadedItemData->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LG::SpawnSingleItem_Async: Failed to load item data or world is invalid"));
		}

		// 6) 현재 아이템 스폰 후 다음 아이템 스폰을 위해 재귀 호출
		SpawnSingleItem_Async();
	}));
}

// Start check level generating time
// if elapsedTime >= MaxGeneratTime => open new level and restart level generating
void APGLevelGenerator::StartDungeonTimer()
{
	// CheckForDungeonComplete();
	UE_LOG(LogTemp, Warning, TEXT("Timer On"));

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandler,
		this,
		&APGLevelGenerator::CheckForDungeonComplete,
		1.0f,
		true,
		0.0f
	);

}

void APGLevelGenerator::CheckForDungeonComplete()
{
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - GenerationStartTime;
	if (ElapsedTime >= MaxGenerateTime)
	{
		UE_LOG(LogTemp, Error, TEXT("LG::CheckForDungeonComplete: Level generation timed out after %.2f seconds. Reloading level."), ElapsedTime);
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%.2f seconds elapsed"), ElapsedTime);
	}
}

APGMasterRoom* APGLevelGenerator::FindFarthestRoom() const
{
	// if no room is created or no starting room, return nullptr
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		return nullptr;
	}

	// queue to store rooms to visit
	TQueue<TObjectPtr<APGMasterRoom>> RoomsToVisit;
	// map to store distance from start room
	TMap<TObjectPtr<APGMasterRoom>, int32> Distances;

	// initiate BFS
	RoomsToVisit.Enqueue(StartRoom);
	Distances.Add(StartRoom, 0);

	// search
	while (!RoomsToVisit.IsEmpty())
	{
		TObjectPtr<APGMasterRoom> CurrentRoom;
		RoomsToVisit.Dequeue(CurrentRoom);

		// finding max distance
		const int32 CurrentDistance = Distances[CurrentRoom];

		// search neighbor rooms connected to the current room
		if (const TArray<TObjectPtr<APGMasterRoom>>* Neighbors = RoomGraph.Find(CurrentRoom))
		{
			for (const TObjectPtr<APGMasterRoom>& Neighbor : *Neighbors)
			{
				// if not visited room
				if (Neighbor && !Distances.Contains(Neighbor))
				{
					// add distance(current distance + 1) to Distances and add room to queue
					Distances.Add(Neighbor, CurrentDistance + 1);
					RoomsToVisit.Enqueue(Neighbor);
				}
			}
		}
	}

	// after search, find farthest room	
	TArray<TObjectPtr<APGMasterRoom>> FarthestRooms;
	int32 MaxDistance = -1;
	for (const auto& Elem : Distances)
	{
		// if current room distance is greater than maximum recorded distance
		if (Elem.Value > MaxDistance)
		{
			// update max distance
			MaxDistance = Elem.Value;

			// empty prior recorded farthest room list
			// add current room as new farthest room
			FarthestRooms.Empty();
			FarthestRooms.Add(Elem.Key);
		}
		// if current room distance is equal to maximum recorded distance
		else if (Elem.Value == MaxDistance)
		{
			// add current room as another farthest room
			FarthestRooms.Add(Elem.Key);
		}
	}

	// random select
	if (!FarthestRooms.IsEmpty())
	{
		int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, FarthestRooms.Num());
		UE_LOG(LogTemp, Warning, TEXT("LG::FindFarthestRoom: random Fartherst room in %d rooms: '%s' (Distance: %d)"), FarthestRooms.Num(), *FarthestRooms[RandomIndex]->GetName(), MaxDistance);
		return FarthestRooms[RandomIndex];
	}	
		
	return nullptr;
}

APGMasterRoom* APGLevelGenerator::FindMiddleDistanceRoom() const
{
	// if no room is created or no starting room, return nullptr
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		return nullptr;
	}

	TQueue<TObjectPtr<APGMasterRoom>> RoomsToVisit;
	TMap<TObjectPtr<APGMasterRoom>, int32> Distances;

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
			for (const TObjectPtr<APGMasterRoom> &Neighbor : *Neighbors)
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

	if (!MiddleDistanceRooms.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, MiddleDistanceRooms.Num());
		UE_LOG(LogTemp, Log, TEXT("LG::FindMiddleDistanceRoom: Middle distance %d rooms, choose '%s' (middle distance: %d, max distance: %d)"), 
			MiddleDistanceRooms.Num(), *MiddleDistanceRooms[RandomIndex]->GetName(), Distances[MiddleDistanceRooms[RandomIndex]], MaxDistance);

		return MiddleDistanceRooms[RandomIndex];
	}

	return nullptr;
}
