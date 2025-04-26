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
#include "PGMasterRoom.h"
#include "PGStartRoom.h"
#include "PGRoom1.h"
#include "PGRoom2.h"
#include "PGRoom3.h"
#include "PGStairRoom1.h"
#include "PGDoor1.h"
#include "PGDoor2.h"
#include "PGDoor3.h"

// Sets default values
APGLevelGenerator::APGLevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Root->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Root->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Root->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	
	RoomsList = {
		APGRoom1::StaticClass(), APGRoom1::StaticClass(),
		APGRoom2::StaticClass(), APGRoom2::StaticClass(),
		APGRoom3::StaticClass(),
	};
		
	SpecialRoomsList = {
		APGStairRoom1::StaticClass()
	};

	BaseRoomsList = {
		APGRoom1::StaticClass(), APGRoom1::StaticClass(),
		APGRoom2::StaticClass(), APGRoom2::StaticClass(),
		APGRoom3::StaticClass(),
	};

	ItemsList = {

	};

	RoomAmount = 10;
	DoorAmount = 3;
	ItemAmount = 0;
	bIsGenerationDone = false;
	MaxGenerateTime = 5.0f;
}

void APGLevelGenerator::SetSeed()
{
	if (SeedValue == -1)
	{
		UKismetMathLibrary::SeedRandomStream(Seed);
	}
	else
	{
		UKismetMathLibrary::SetRandomStreamSeed(Seed, SeedValue);
	}
}

void APGLevelGenerator::SpawnStartRoom()
{
	UWorld* world = GetWorld();
	if (world)
	{
		FTransform spawnTransform = Root->GetComponentTransform();
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APGStartRoom* newRoom = world->SpawnActor<APGStartRoom>(APGStartRoom::StaticClass(), spawnTransform, spawnParams);

		TArray<USceneComponent*> exitsItem;
		newRoom->GetExitsFolder()->GetChildrenComponents(false, exitsItem);
		ExitsList.Append(exitsItem);
	}
}

void APGLevelGenerator::SpawnNextRoom()
{
	UWorld* world = GetWorld();
	if (world)
	{
		// get random item from list by stream
		SelectedExitPoint = ExitsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, ExitsList.Num())];
		TSubclassOf<APGMasterRoom> newRoomClass = RoomsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, RoomsList.Num())];

		FVector spawnLocation = SelectedExitPoint->GetComponentLocation();
		FRotator spawnRotation = SelectedExitPoint->GetComponentRotation();
		FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APGMasterRoom* newRoom = world->SpawnActor<APGMasterRoom>(newRoomClass, spawnTransform, spawnParams);
		LatestRoom = newRoom;

		CheckOverlap();
	}
}

void APGLevelGenerator::AddOverlappingRoomsToList()
{
	TArray<USceneComponent*> overlapItems;
	LatestRoom->GetOverlapBoxFolder()->GetChildrenComponents(false, overlapItems);
	for (auto arrayItem : overlapItems)
	{
		UBoxComponent* overlapItem = Cast<UBoxComponent>(arrayItem);
		TArray<UPrimitiveComponent*> temp;
		overlapItem->GetOverlappingComponents(temp);
		OverlappedList.Append(temp);
	}
}

void APGLevelGenerator::AddFloorSpawnPointsToList()
{
	TArray<USceneComponent*> floorSpawnPoints;
	LatestRoom->GetFloorSpawnPointsFolder()->GetChildrenComponents(false, floorSpawnPoints);
	FloorSpawnPointsList.Append(floorSpawnPoints);
}

void APGLevelGenerator::CheckOverlap()
{
	AddOverlappingRoomsToList();
	if (!OverlappedList.IsEmpty())
	{
		OverlappedList.Empty();
		LatestRoom->Destroy();
		SpawnNextRoom();
	}
	else
	{
		OverlappedList.Empty();
		// remove selected exit point from exits list and add to door points list
		ExitsList.Remove(SelectedExitPoint);
		DoorPointsList.Add(SelectedExitPoint);

		RoomAmount--;

		TArray<USceneComponent*> latestRoomExitPoints;
		LatestRoom->GetExitsFolder()->GetChildrenComponents(false, latestRoomExitPoints);
		ExitsList.Append(latestRoomExitPoints);
		AddFloorSpawnPointsToList();

		if (RoomAmount > 0)
		{
			if (RoomAmount == 4 || RoomAmount == 8)
			{
				RoomsList = SpecialRoomsList;
				SpawnNextRoom();
			}
			else
			{
				RoomsList = BaseRoomsList;
				SpawnNextRoom();
			}
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandler);
			CloseHoles();
			SpawnDoors();
			SpawnItems();
			bIsGenerationDone = true;
		}
	}
}

void APGLevelGenerator::CloseHoles()
{
	UWorld* world = GetWorld();
	if (world)
	{
		for (auto exitPoint : ExitsList)
		{
			FVector spawnLocation = exitPoint->GetComponentLocation();
			FRotator spawnRotation = exitPoint->GetComponentRotation();
			spawnRotation.Yaw += 90.0f;
			FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			UArrowComponent* CastedSelectedDoorPoint = Cast<UArrowComponent>(exitPoint);
			if (CastedSelectedDoorPoint->ArrowLength == 80.0f)
			{
				world->SpawnActor<APGDoor1>(APGDoor1::StaticClass(), spawnTransform, spawnParams);
			}
			else if (CastedSelectedDoorPoint->ArrowLength == 80.1f)
			{
				world->SpawnActor<APGDoor2>(APGDoor2::StaticClass(), spawnTransform, spawnParams);
			}
			else
			{
				world->SpawnActor<APGDoor3>(APGDoor3::StaticClass(), spawnTransform, spawnParams);
			}
		}
	}
}

void APGLevelGenerator::SpawnDoors()
{
	UWorld* world = GetWorld();
	if (world)
	{
		while (DoorAmount > 0)
		{
			SelectedDoorPoint = DoorPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, DoorPointsList.Num())];
			FVector spawnLocation = SelectedDoorPoint->GetComponentLocation();
			FRotator spawnRotation = SelectedDoorPoint->GetComponentRotation();
			spawnRotation.Yaw += 90.0f;
			FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			UArrowComponent* CastedSelectedDoorPoint = Cast<UArrowComponent>(SelectedDoorPoint);
			if (CastedSelectedDoorPoint->ArrowLength == 80.0f)
			{
				world->SpawnActor<APGDoor1>(APGDoor1::StaticClass(), spawnTransform, spawnParams);
			}
			else if (CastedSelectedDoorPoint->ArrowLength == 80.1f)
			{
				world->SpawnActor<APGDoor2>(APGDoor2::StaticClass(), spawnTransform, spawnParams);
			}
			else
			{
				world->SpawnActor<APGDoor3>(APGDoor3::StaticClass(), spawnTransform, spawnParams);
			}

			DoorPointsList.Remove(SelectedDoorPoint);
			DoorAmount--;
		}
	}
}

void APGLevelGenerator::SpawnItems()
{
	UWorld* world = GetWorld();
	if (world)
	{
		while (ItemAmount > 0)
		{
			SelectedFloorSpawnPoint = FloorSpawnPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, FloorSpawnPointsList.Num())];
			TSubclassOf<AActor> newItemClass = ItemsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, ItemsList.Num())];

			FVector spawnLocation = SelectedFloorSpawnPoint->GetComponentLocation();
			FTransform spawnTransform(FRotator(0.0f, 0.0f, 0.0f), spawnLocation, FVector(1.0f, 1.0f, 1.0f));
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			world->SpawnActor<AActor>(newItemClass, spawnTransform, spawnParams);

			FloorSpawnPointsList.Remove(SelectedFloorSpawnPoint);
			ItemAmount--;
		}
	}
}

void APGLevelGenerator::StartDungeonTimer()
{
	// CheckForDungeonComplete();

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
	UE_LOG(LogTemp, Warning, TEXT("Timer On"));
	float elapsedTime = GetWorld()->GetTimeSeconds() - GenerationStartTime;

	if (elapsedTime >= MaxGenerateTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("Time Over"));
		GetWorld()->GetTimerManager().ClearTimer(TimerHandler);
		UGameplayStatics::OpenLevel(this, FName("LV_PGMainLevel"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%.2f seconds elapsed"), elapsedTime);
	}
}

// Called when the game starts or when spawned
void APGLevelGenerator::BeginPlay()
{
	Super::BeginPlay();
	SetSeed();
	SpawnStartRoom();

	GenerationStartTime = GetWorld()->GetTimeSeconds();
	StartDungeonTimer();

	SpawnNextRoom();	
}
