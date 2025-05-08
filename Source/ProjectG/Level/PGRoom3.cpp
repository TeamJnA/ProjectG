// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom3.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGRoom3::APGRoom3()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/BP/LI_LabSample.LI_LabSample'"));

	SpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint0"));
	SpawnPoint0->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint0->SetRelativeLocation(FVector(2350.0f, 620.0f, 92.0f));
	SpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint0->bHiddenInGame = false;

	SpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint1"));
	SpawnPoint1->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint1->SetRelativeLocation(FVector(3125.0f, 290.0f, 92.0f));
	SpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint1->bHiddenInGame = false;

	SpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint2"));
	SpawnPoint2->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint2->SetRelativeLocation(FVector(3250.0f, -290.0f, 92.0f));
	SpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint2->bHiddenInGame = false;

	SpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint3"));
	SpawnPoint3->SetupAttachment(FloorSpawnPointsFolder);
	SpawnPoint3->SetRelativeLocation(FVector(2260.0f, -290.0f, 92.0f));
	SpawnPoint3->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	SpawnPoint3->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SpawnPoint3->bHiddenInGame = false;

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1670.0f, 0.0f, 220.0f));
	OverlapBox->SetRelativeScale3D(FVector(51.0f, 21.0f, 6.5f));
	
	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1674.0f, 0.0f, -8.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
