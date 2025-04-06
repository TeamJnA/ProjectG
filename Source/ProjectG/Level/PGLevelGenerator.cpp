// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLevelGenerator.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PGMasterRoom.h"
#include "PGStartRoom.h"
#include "PGRoom1.h"
#include "PGRoom2.h"
#include "PGRoom3.h"
#include "PGRoom4.h"
#include "PGStairRoom1.h"
#include "PGHallway.h"
#include "PGWall.h"
#include "PGDoor1.h"

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
		APGRoom1::StaticClass(), APGRoom1::StaticClass(), APGRoom1::StaticClass(),
		APGRoom2::StaticClass(),
		APGRoom3::StaticClass(),
		APGRoom4::StaticClass(),
		APGHallway::StaticClass(), APGHallway::StaticClass(), APGHallway::StaticClass(), APGHallway::StaticClass()
	};

	SpecialRoomsList = {
		APGStairRoom1::StaticClass()
	};

	BaseRoomsList = {
		APGRoom1::StaticClass(), APGRoom1::StaticClass(), APGRoom1::StaticClass(),
		APGRoom2::StaticClass(),
		APGRoom3::StaticClass(),
		APGRoom4::StaticClass(),
		APGHallway::StaticClass(), APGHallway::StaticClass(), APGHallway::StaticClass(), APGHallway::StaticClass()
	};

	RoomAmount = 40;
	DoorAmount = 8;
	bIsGenerationDone = false;
	MaxGenerateTime = 40.0f;
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
	// get random item from list by stream
	SelectedExitPoint = ExitsList[UKismetMathLibrary::RandomIntegerFromStream(ExitsList.Num(), Seed)];
	TSubclassOf<AActor> newRoomClass = RoomsList[UKismetMathLibrary::RandomIntegerFromStream(RoomsList.Num(), Seed)];

	UWorld* world = GetWorld();
	if (world)
	{
		FVector spawnLocation = SelectedExitPoint->GetComponentLocation();
		FRotator spawnRotation = SelectedExitPoint->GetComponentRotation();
		FTransform spawnTransform(spawnRotation, spawnLocation, FVector(1.0f, 1.0f, 1.0f));
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* newRoom = world->SpawnActor<AActor>(newRoomClass, spawnTransform, spawnParams);
		LatestRoom = Cast<APGMasterRoom>(newRoom);

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

		if (RoomAmount > 0)
		{
			if (RoomAmount == 30 || RoomAmount == 20)
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
			CloseHoles();
			SpawnDoors();

			bIsGenerationDone = true;
			GetWorld()->GetTimerManager().ClearTimer(TimerHandler);
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
			FTransform spawnTransform = exitPoint->GetComponentTransform();
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			world->SpawnActor<APGWall>(APGWall::StaticClass(), spawnTransform, spawnParams);
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
			SelectedDoorPoint = DoorPointsList[UKismetMathLibrary::RandomIntegerFromStream(DoorPointsList.Num(), Seed)];
			FTransform spawnTransform = SelectedDoorPoint->GetComponentTransform();
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			world->SpawnActor<APGDoor1>(APGDoor1::StaticClass(), spawnTransform, spawnParams);

			DoorPointsList.Remove(SelectedDoorPoint);
			DoorAmount--;
		}
	}
}

void APGLevelGenerator::StartDungeonTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandler,
		this,
		&APGLevelGenerator::CheckForDungeonComplete,
		1.0f,
		true
	);
}

void APGLevelGenerator::CheckForDungeonComplete()
{
	if (GetWorld()->GetTimeSeconds() >= MaxGenerateTime)
	{
		UGameplayStatics::OpenLevel(this, FName("LV_PGMainLevel"));
	}
}

// Called when the game starts or when spawned
void APGLevelGenerator::BeginPlay()
{
	Super::BeginPlay();
	SetSeed();
	SpawnStartRoom();
	StartDungeonTimer();
	SpawnNextRoom();	
}
