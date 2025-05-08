// Fill out your copyright notice in the Description page of Project Settings.


#include "PGStartRoom.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGStartRoom::APGStartRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/BP/LI_StartRoomSample.LI_StartRoomSample'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1422.0f, -1413.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(2148.0f, 0.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->SetArrowLength(80.2f);
	ExitDir1->bHiddenInGame = false;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(1422.0f, 1403.0f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->SetArrowLength(80.1f);
	ExitDir2->bHiddenInGame = false;

	SpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint0"));
	SpawnPoint0->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint0->SetRelativeLocation(FVector(700.0f, -880.0f, 13.0f));
	SpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint0->bHiddenInGame = false;

	SpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint1"));
	SpawnPoint1->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint1->SetRelativeLocation(FVector(900.0f, -950.0f, 13.0f));
	SpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint1->bHiddenInGame = false;

	SpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint2"));
	SpawnPoint2->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint2->SetRelativeLocation(FVector(900.0f, 940.0f, 13.0f));
	SpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint2->bHiddenInGame = false;

	SpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint3"));
	SpawnPoint3->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint3->SetRelativeLocation(FVector(700.0f, 875.0f, 13.0f));
	SpawnPoint3->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint3->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint3->bHiddenInGame = false;

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1075.0f, 0.0f, 240.0f));
	OverlapBox->SetRelativeScale3D(FVector(32.5f, 43.5f, 11.75f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1094.8f, -5.0f, -170.3f));
}
