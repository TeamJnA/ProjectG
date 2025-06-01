// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGRoom1::APGRoom1()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_MansionCorridor.LI_MansionCorridor_C'"));
		
	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1540.0f, -365.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(2740.0f, -365.0f, 0.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = false;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(3669.0f, -11.0f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = false;

	ExitDir3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection3"));
	ExitDir3->SetupAttachment(ExitsFolder);
	ExitDir3->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir3->SetRelativeLocation(FVector(2140.0f, 339.0f, 0.0f));
	ExitDir3->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = false;

	ExitDir4 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection4"));
	ExitDir4->SetupAttachment(ExitsFolder);
	ExitDir4->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir4->SetRelativeLocation(FVector(940.0f, 339.0f, 0.0f));
	ExitDir4->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir4->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir4->bHiddenInGame = false;

	SpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint0"));
	SpawnPoint0->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint0->SetRelativeLocation(FVector(1154.0f, -182.0f, 81.5f));
	SpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint0->bHiddenInGame = false;

	SpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint1"));
	SpawnPoint1->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint1->SetRelativeLocation(FVector(1244.0f, -182.0f, 81.5f));
	SpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint1->bHiddenInGame = false;

	SpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint2"));
	SpawnPoint2->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint2->SetRelativeLocation(FVector(1340.0f, -182.0f, 81.5f));
	SpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint2->bHiddenInGame = false;

	SpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint3"));
	SpawnPoint3->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint3->SetRelativeLocation(FVector(1424.0f, -182.0f, 81.5f));
	SpawnPoint3->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint3->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint3->bHiddenInGame = false;

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1835.0f, -15.0f, 310.0f));
	OverlapBox->SetRelativeScale3D(FVector(57.0f, 10.8f, 9.25f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1853.0f, -15.0f, -49.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
