// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLevelGenerator.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
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

#include "PGGlobalLightManager.h"
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

	Root->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Root->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Root->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	
	RoomsList = {
		APGRoom2::StaticClass(), APGRoom2::StaticClass(),
		APGRoom3::StaticClass(),
		APGStairRoom1::StaticClass()
	};
		
	//SpecialRoomsList = {
	//	APGStairRoom1::StaticClass()
	//};

	//BaseRoomsList = {
	//	APGRoom1::StaticClass(), APGRoom1::StaticClass(),
	//	APGRoom2::StaticClass(), APGRoom2::StaticClass(),
	//	APGRoom3::StaticClass(),
	//};

	// max room spawn amount
	RoomAmount = 20;
	// max door spawn amount
	// door: created between rooms
	// wall: created between room and outside
	DoorAmount = 0;
	LockedDoorAmount = 0;

	// max item spawn amount
	ItemAmount = 20;

	bIsGenerationDone = false;

	// reload level if (elpased time > max generation time)
	MaxGenerateTime = 8.0f;
}

// set level seed
// All actors are spawned based on seeds
// If seed value == -1, generate random seed
// If a specific value is specified, set that value as seed.
void APGLevelGenerator::SetSeed()
{
	if (HasAuthority())
	{
		if (SeedValue == -1)
		{
			Seed.Initialize(FDateTime::Now().GetTicks());
			//UKismetMathLibrary::SeedRandomStream(Seed);
		}
		else
		{
			Seed.Initialize(SeedValue);
			//UKismetMathLibrary::SetRandomStreamSeed(Seed, SeedValue);
		}
	}
}

// Create the base room, PGStartRoom
void APGLevelGenerator::SpawnStartRoom()
{
	UWorld* world = GetWorld();
	if (world)
	{
		// spawn setting
		FTransform spawnTransform = Root->GetComponentTransform();
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APGStartRoom* newRoom = world->SpawnActor<APGStartRoom>(APGStartRoom::StaticClass(), spawnTransform, spawnParams);

		// spawn default key item
		// get game instance for use PGItemData instances
		UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(world->GetGameInstance());

		APGItemActor* DefaultKey1 = world->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), spawnParams);
		if (UPGItemData* ItemData1 = GI->GetItemDataByKey("Key"))
		{
			DefaultKey1->InitWithData(ItemData1);
		}
		DefaultKey1->SetActorRelativeLocation(FVector(538.0f, 271.0f, 90.0f));

		APGItemActor* DeafultKey2 = world->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), spawnParams);
		if (UPGItemData* ItemData2 = GI->GetItemDataByKey("Key"))
		{
			DeafultKey2->InitWithData(ItemData2);
		}
		DeafultKey2->SetActorRelativeLocation(FVector(538.0f, 356.0f, 90.0f));

		APGExitDoor* ExitDoor = world->SpawnActor<APGExitDoor>(APGExitDoor::StaticClass(), spawnParams);
		ExitDoor->SetActorRelativeLocation(FVector(1850.0f, 317.0f, 10.0f));
		ExitDoor->SetActorRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

		// Add the Points(exitpoints) where the next Room can be created to the ExitsList.
		TArray<USceneComponent*> exitsItem;
		newRoom->GetExitsFolder()->GetChildrenComponents(false, exitsItem);
		ExitsList.Append(exitsItem);
	}
}

// Create Rooms repeatedly a fixed number of times(RoomAmount)
void APGLevelGenerator::SpawnNextRoom()
{
	UWorld* world = GetWorld();
	if (world)
	{
		// get random room spawn point from ExitsList by stream(seed)
		SelectedExitPoint = ExitsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, ExitsList.Num())];
		// get random class of room type from RoomsList by stream(seed)

		// spawn setting
		FVector spawnLocation = SelectedExitPoint->GetComponentLocation();
		FRotator spawnRotation = SelectedExitPoint->GetComponentRotation();
		FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APGMasterRoom* newRoom = nullptr;

		if (RoomAmount > 14)
		{
			newRoom = world->SpawnActor<APGMasterRoom>(APGRoom1::StaticClass(), spawnTransform, spawnParams);
		}
		else
		{
			TSubclassOf<APGMasterRoom> newRoomClass = RoomsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, RoomsList.Num())];
			newRoom = world->SpawnActor<APGMasterRoom>(newRoomClass, spawnTransform, spawnParams);
		}

		// set spawned room as LatestRoom
		LatestRoom = newRoom;

		// Check if the created room does not overlap with another room
		// Check if the created room exceeds the room amount
		// CheckOverlap();

		// Delay before spawn next room
		GetWorld()->GetTimerManager().SetTimer(
			DelayTimerHandler,
			this,
			&APGLevelGenerator::CheckOverlap,
			0.1f,
			false
		);
	}
}

// Check if there is another room overlapping the OverlapBox of LatestRoom(most recently spawned room)
void APGLevelGenerator::AddOverlappingRoomsToList()
{
	TArray<USceneComponent*> overlapItems;
	// OverlapItems == OverlapBoxes of LatestRoom
	LatestRoom->GetOverlapBoxFolder()->GetChildrenComponents(false, overlapItems);
	for (auto arrayItem : overlapItems)
	{
		// use cast USceneComponent(arrayItem) -> UBoxComponent(overlapItem) because UBoxComponent can't use GetChildrenComponents()
		UBoxComponent* overlapItem = Cast<UBoxComponent>(arrayItem);
		TArray<UPrimitiveComponent*> temp;
		// get overlapping components of overlapItem and append them to temp array
		// use UPrimitiveComponent because GetOverlappingComponents(TArray<UPrimitiveComponent*> arr)
		// after add all overlapping components to temp then move them to OverlappedList
		overlapItem->GetOverlappingComponents(temp);
		OverlappedList.Append(temp);
	}
}

// Add item spawn points of LatestRoom(most recently spawned room) to FloorSpawnPointsList
void APGLevelGenerator::AddFloorSpawnPointsToList()
{
	TArray<USceneComponent*> floorSpawnPoints;
	LatestRoom->GetFloorSpawnPointsFolder()->GetChildrenComponents(false, floorSpawnPoints);
	FloorSpawnPointsList.Append(floorSpawnPoints);
}

// 1) Check if there is another room overlapping the OverlapBox of LatestRoom(most recently spawned room)
// 2) Check can generate next room
void APGLevelGenerator::CheckOverlap()
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
	// if there is no overlapped room
	else
	{
		// empty OverlappedList for next spawn room
		// remove SelectedExitPoint(LatestRoom's spawn point) from exits list and add to door points list
		OverlappedList.Empty();
		ExitsList.Remove(SelectedExitPoint);
		DoorPointsList.Add(SelectedExitPoint);

		// update RoomAmount
		RoomAmount--;

		// add LatestRoom's exitpoints(where the next Room can be spawned) to the ExitsList
		TArray<USceneComponent*> latestRoomExitPoints;
		LatestRoom->GetExitsFolder()->GetChildrenComponents(false, latestRoomExitPoints);
		ExitsList.Append(latestRoomExitPoints);
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
	GetWorld()->GetTimerManager().ClearTimer(TimerHandler);
	GetWorld()->GetTimerManager().ClearTimer(DelayTimerHandler);
	UE_LOG(LogTemp, Warning, TEXT("Generate Rooms Completely"));

	CloseHoles();
	SpawnDoors();
	SpawnItems();
	bIsGenerationDone = true;

	// Called GameState and inside GS, trigger a delegate to notify GameMode to spawn the player.
	if (GetWorld())
	{
		if (APGGameState* gs = GetWorld()->GetGameState<APGGameState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("LevelGenerator: Notify map generation complete to GameState"));
			gs->NotifyMapGenerationComplete();
		}
	}
}

// spawn walls(created between room and outside)
void APGLevelGenerator::CloseHoles()
{
	UWorld* world = GetWorld();
	if (world)
	{
		// get all exit point from ExitsList then spawn wall actor
		for (auto exitPoint : ExitsList)
		{
			// spawn setting
			FVector spawnLocation = exitPoint->GetComponentLocation();
			FRotator spawnRotation = exitPoint->GetComponentRotation();
			spawnRotation.Yaw += 90.0f;
			FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			world->SpawnActor<APGWall>(APGWall::StaticClass(), spawnTransform, spawnParams);
		}
	}
}

// spawn doors(created between rooms) by a certain number(DoorAmount)
void APGLevelGenerator::SpawnDoors()
{
	UWorld* world = GetWorld();
	if (world)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] DoorPointsList Contents:"), HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
		for (auto Point : DoorPointsList)
		{
			UE_LOG(LogTemp, Warning, TEXT("Point: %s"), *Point->GetComponentLocation().ToString());
		}

		DoorAmount = DoorPointsList.Num() * 0.8f;
		LockedDoorAmount = DoorAmount * 0.3f;

		while (DoorAmount > 0)
		{
			// get random door spawn point from DoorPointsList by stream(seed)
			SelectedDoorPoint = DoorPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, DoorPointsList.Num())];

			// spawn setting
			FVector SpawnLocation = SelectedDoorPoint->GetComponentLocation();
			FRotator SpawnRotation = SelectedDoorPoint->GetComponentRotation();
			FTransform SpawnTransform(SpawnRotation, SpawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.bNoFail = true;

			if (LockedDoorAmount > 0)
			{
				APGDoor1::SpawnDoor(world, SpawnTransform, SpawnParams, /*bIsLocked*/ true);
				LockedDoorAmount--;
			}
			else
			{
				APGDoor1::SpawnDoor(world, SpawnTransform, SpawnParams, /*bIsLocked*/ false);
			}

			// after spawn door, remove used spawn point from DoorPointsList and DoorAmount--
			DoorPointsList.Remove(SelectedDoorPoint);
			DoorAmount--;
		}
	}
}

// spawn items by a certain number(ItemAmount)
void APGLevelGenerator::SpawnItems()
{
	SpawnSingleItem_Async();

	//UWorld* world = GetWorld();
	//if (world)
	//{
	//	while (ItemAmount > 0)
	//	{
	//		// get random item spawn point from FloorSpawnPointsList by stream(seed)
	//		SelectedFloorSpawnPoint = FloorSpawnPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, FloorSpawnPointsList.Num())];
	//		
	//		// spawn setting
	//		FVector spawnLocation = SelectedFloorSpawnPoint->GetComponentLocation();
	//		FTransform spawnTransform(FRotator(0.0f, 0.0f, 0.0f), spawnLocation, FVector(1.0f, 1.0f, 1.0f));
	//		FActorSpawnParameters spawnParams;
	//		spawnParams.Owner = this;
	//		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//		// get game instance for use PGItemData instances
	//		UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(world->GetGameInstance());
	//		
	//		APGItemActor* newItem = world->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), spawnTransform, spawnParams);

	//		//UE_LOG(LogTemp, Warning, TEXT("SERVER SPAWNED Item: Name=%s, Location=%s, NetGUID=%s"), *newItem->GetFName().ToString(), *newItem->GetActorLocation().ToString(), *newItem->GetActorGuid().ToString()); //

	//		// spawn items that must be spawned(Item_Escape) (ItemAmount == 10, 9, 8)
	//		// other items spawn randomly
	//		// InitWithData => spawn item actor's class(PGItemActor) then attach actual data(PGItemData) for item
	//		if (ItemAmount >= 18)
	//		{
	//			if (UPGItemData* itemData = GI->GetItemDataByKey("ExitKey"))
	//			{
	//				newItem->InitWithData(itemData);
	//			}
	//		}
	//		else if (ItemAmount >= 10)
	//		{
	//			if (UPGItemData* itemData = GI->GetItemDataByKey("Brick"))
	//			{
	//				newItem->InitWithData(itemData);
	//			}
	//		}
	//		else
	//		{
	//			if (UPGItemData* itemData = GI->GetItemDataByKey("Key"))
	//			{
	//				newItem->InitWithData(itemData);
	//			}
	//		}

	//		// after spawn item, remove used spawn point from FloorSpawnPointsList and ItemAmount--
	//		FloorSpawnPointsList.Remove(SelectedFloorSpawnPoint);
	//		ItemAmount--;
	//	}
	//}
}

void APGLevelGenerator::SpawnSingleItem_Async()
{
	UWorld* World = GetWorld();
	// 아이템을 모두 스폰했거나, 스폰 위치가 더이상 없거나, world가 없으면 중단
	if (!World || ItemAmount <= 0 || FloorSpawnPointsList.Num() <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("LG::SpawnSingleItem_Async: Item spawning process finished or stopped"));
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("LG::SpawnSingleItem_Async: GI is not valid"));
	}

	// 1) 스폰 위치 선정 (Seed 기반)
	int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, FloorSpawnPointsList.Num());
	SelectedFloorSpawnPoint = FloorSpawnPointsList[RandomIndex];
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
	FVector SpawnLocation = SelectedFloorSpawnPoint->GetComponentLocation();
	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);
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
	float elapsedTime = GetWorld()->GetTimeSeconds() - GenerationStartTime;

	if (elapsedTime >= MaxGenerateTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("Time Over"));

		GetWorld()->GetTimerManager().ClearTimer(DelayTimerHandler);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandler);
		UE_LOG(LogTemp, Warning, TEXT("Stop Map Generating"));

		UE_LOG(LogTemp, Warning, TEXT("Reboot Level"));

		/*
		* On level generate failed
		* Initiate travel timer(GS->NotifyServerTravel) & ServerTravel
		*/
		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
		if (!GS) return;
		GS->NotifyStartTravel();

		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
		{
			GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
		}));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%.2f seconds elapsed"), elapsedTime);
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
