// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom2.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGRoom2::APGRoom2()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_Room2.LI_Room2_C'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1500.0f, -1190.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(5160.0f, 0.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->SetArrowLength(80.0f);
	ExitDir1->bHiddenInGame = false;

	SpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint0"));
	SpawnPoint0->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint0->SetRelativeLocation(FVector(1825.0f, 610.0f, 91.0f));
	SpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint0->bHiddenInGame = false;

	SpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint1"));
	SpawnPoint1->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint1->SetRelativeLocation(FVector(1945.0f, 610.0f, 91.0f));
	SpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint1->bHiddenInGame = false;

	SpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint2"));
	SpawnPoint2->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint2->SetRelativeLocation(FVector(2065.0f, 610.0f, 91.0f));
	SpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint2->bHiddenInGame = false;

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(2580.0f, -160.0f, 220.0f));
	OverlapBox->SetRelativeScale3D(FVector(78.75f, 31.0f, 6.5f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(2580.0f, -175.6f, -125.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}
