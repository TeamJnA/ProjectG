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
	ItemAmount = 10;

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
			//ReplicatedSeedValue = Seed.GetCurrentSeed();
			//UKismetMathLibrary::SeedRandomStream(Seed);
		}
		else
		{
			Seed.Initialize(SeedValue);
			//ReplicatedSeedValue = SeedValue;
			//UKismetMathLibrary::SetRandomStreamSeed(Seed, SeedValue);
		}

		//UE_LOG(LogTemp, Warning, TEXT("[SERVER] APGLevelGenerator Seed: %d"), ReplicatedSeedValue);
	}
	else
	{
		// 클라이언트는 이미 복제된 ReplicatedSeedValue를 사용하여 자신의 스트림을 초기화합니다.
		//Seed.Initialize(ReplicatedSeedValue);
		// 디버깅을 위해 클라이언트에서 설정된 시드 값을 로그로 남길 수 있습니다.
		//UE_LOG(LogTemp, Warning, TEXT("[CLIENT] APGLevelGenerator Seed Replicated: %d"), ReplicatedSeedValue);
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
			//// next room == special room(PGStairRoom)
			//if (RoomAmount == 4 || RoomAmount == 8)
			//{
			//	RoomsList = SpecialRoomsList;
			//	SpawnNextRoom();
			//}
			//// next room == base room(PGRoom1 ~ PGRoom3)
			//else
			//{
			//	RoomsList = BaseRoomsList;
			//	SpawnNextRoom();
			//}
			SpawnNextRoom();
		}
		// stop generation => spawn walls, doors, items and clear timer, spawn global light manager(PGGlobalLightManager)
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandler);
			GetWorld()->GetTimerManager().ClearTimer(DelayTimerHandler);
			UE_LOG(LogTemp, Warning, TEXT("Done"));

			CloseHoles();
			SpawnDoors();
			SpawnItems();
			bIsGenerationDone = true;

			if (GetWorld())
			{
				if (APGGameState* gs = GetWorld()->GetGameState<APGGameState>())
				{	
					UE_LOG(LogTemp, Warning, TEXT("LevelGenerator: Notify map generation complete to GameState"));
					gs->NotifyMapGenerationComplete();
				}
			}
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

		int32 DoorIndex = 0; // 문 인덱스를 추가하여 고유한 이름 생성
		while (DoorAmount > 0)
		{
			// get random door spawn point from DoorPointsList by stream(seed)
			SelectedDoorPoint = DoorPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, DoorPointsList.Num())];

			// spawn setting
			FVector spawnLocation = SelectedDoorPoint->GetComponentLocation();
			FRotator spawnRotation = SelectedDoorPoint->GetComponentRotation();
			//spawnRotation.Yaw += 90.0f;
			FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			//UE_LOG(LogTemp, Warning, TEXT("SERVER SPAWNING DOOR: Owner NetGUID=%s"), *spawnParams.Owner->GetActorGuid().ToString());

			APGDoor1* NewDoor = world->SpawnActor<APGDoor1>(APGDoor1::StaticClass(), spawnTransform, spawnParams);

			if (NewDoor)
			{
				if (LockedDoorAmount > 0)
				{
					NewDoor->Lock();
					LockedDoorAmount--;
				}
				else
				{
					// bIsLock deafult = false
				}
				//UE_LOG(LogTemp, Warning, TEXT("SERVER SPAWNED DOOR: Name=%s, Location=%s, NetGUID=%s"), *NewDoor->GetFName().ToString(), *NewDoor->GetActorLocation().ToString(), *NewDoor->GetActorGuid().ToString()); //
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn APGDoor1 from APGLevelGenerator::SpawnDoors!")); // 에러 로그 추가
			}

			//if (LockedDoorAmount > 0)
			//{
			//	APGDoor1::SpawnDoor(world, spawnTransform, spawnParams, /*bIsLocked*/ true);
			//	LockedDoorAmount--;
			//}
			//else
			//{
			//	APGDoor1::SpawnDoor(world, spawnTransform, spawnParams, /*bIsLocked*/ false);
			//}

			// spawn doors to fit the size of the hole
			// categorized hole size by exitpoint's ArrowLength
			//UArrowComponent* CastedSelectedDoorPoint = Cast<UArrowComponent>(SelectedDoorPoint);
			//if (CastedSelectedDoorPoint->ArrowLength == 80.0f)
			//{
			//	world->SpawnActor<APGDoor1>(APGDoor1::StaticClass(), spawnTransform, spawnParams);
			//}
			//else if (CastedSelectedDoorPoint->ArrowLength == 80.1f)
			//{
			//	world->SpawnActor<APGDoor2>(APGDoor2::StaticClass(), spawnTransform, spawnParams);
			//}
			//else
			//{
			//	world->SpawnActor<APGDoor3>(APGDoor3::StaticClass(), spawnTransform, spawnParams);
			//}

			// after spawn door, remove used spawn point from DoorPointsList and DoorAmount--
			DoorPointsList.Remove(SelectedDoorPoint);
			DoorAmount--;
			DoorIndex++; // 다음 문을 위해 인덱스 증가
		}
	}
}

// spawn items by a certain number(ItemAmount)
void APGLevelGenerator::SpawnItems()
{
	UWorld* world = GetWorld();
	if (world)
	{
		while (ItemAmount > 0)
		{
			// get random item spawn point from FloorSpawnPointsList by stream(seed)
			SelectedFloorSpawnPoint = FloorSpawnPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, FloorSpawnPointsList.Num())];
			
			// spawn setting
			FVector spawnLocation = SelectedFloorSpawnPoint->GetComponentLocation();
			FTransform spawnTransform(FRotator(0.0f, 0.0f, 0.0f), spawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// get game instance for use PGItemData instances
			UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(world->GetGameInstance());
			
			APGItemActor* newItem = world->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), spawnTransform, spawnParams);

			//UE_LOG(LogTemp, Warning, TEXT("SERVER SPAWNED Item: Name=%s, Location=%s, NetGUID=%s"), *newItem->GetFName().ToString(), *newItem->GetActorLocation().ToString(), *newItem->GetActorGuid().ToString()); //

			// spawn items that must be spawned(Item_Escape) (ItemAmount == 10, 9, 8)
			// other items spawn randomly
			// InitWithData => spawn item actor's class(PGItemActor) then attach actual data(PGItemData) for item
			if (ItemAmount == 10)
			{
				if (UPGItemData* itemData = GI->GetItemDataByKey("AdminDevice"))
				{
					newItem->InitWithData(itemData);
				}
			}
			else if (ItemAmount == 9)
			{
				if (UPGItemData* itemData = GI->GetItemDataByKey("EnergyCore"))
				{
					newItem->InitWithData(itemData);
				}
			}
			else if (ItemAmount == 8)
			{
				if (UPGItemData* itemData = GI->GetItemDataByKey("RootCalculator"))
				{
					newItem->InitWithData(itemData);
				}
			}
			else
			{
				if (UPGItemData* itemData = GI->GetItemDataByKey("Brick"))
				{
					newItem->InitWithData(itemData);
				}
			}

			// after spawn item, remove used spawn point from FloorSpawnPointsList and ItemAmount--
			FloorSpawnPointsList.Remove(SelectedFloorSpawnPoint);
			ItemAmount--;
		}
	}
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

	//if (HasAuthority())
	//{
	//	SetSeed();
	//	SpawnStartRoom();

	//	GenerationStartTime = GetWorld()->GetTimeSeconds();
	//	StartDungeonTimer();

	//	SpawnNextRoom();
	//}

	if (HasAuthority())
	{
		FTimerHandle WaitingTimer;
		GetWorld()->GetTimerManager().SetTimer(WaitingTimer, FTimerDelegate::CreateLambda([this]()
		{
            UE_LOG(LogTemp, Warning, TEXT("2sec"));

			//UE_LOG(LogTemp, Warning, TEXT("[SERVER] APGLevelGenerator BeginPlay. Role: %s, NetGUID: %s"), *UEnum::GetValueAsString(GetLocalRole()), *GetActorGuid().ToString());
			SetSeed();
			SpawnStartRoom();

			GenerationStartTime = GetWorld()->GetTimeSeconds();
			StartDungeonTimer();

			SpawnNextRoom();
			//UE_LOG(LogTemp, Warning, TEXT("[SERVER] APGLevelGenerator BeginPlay. Role: %s, RemoteRole: %s, NetGUID: %s"), *UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(GetRemoteRole()), *GetActorGuid().ToString());
		}), 10.0f, false);
	}
	else // 클라이언트에서 APGLevelGenerator가 복제되었다면
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CLIENT] APGLevelGenerator BeginPlay. Role: %s, RemoteRole: %s, NetGUID: %s"), *UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(GetRemoteRole()), *GetActorGuid().ToString());
	}
}

//void APGLevelGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(APGLevelGenerator, ReplicatedSeedValue); // <-- Seed 변수를 복제 목록에 추가합니다.
//	DOREPLIFETIME(APGLevelGenerator, DoorPointsList);
//}
//
//void APGLevelGenerator::OnRep_ReplicatedSeedValue()
//{
//	SetSeed();
//}
