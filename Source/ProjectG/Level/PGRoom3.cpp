// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom3.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGRoom3::APGRoom3()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_MansionBedroom.LI_MansionBedroom_C'"));

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(138.0f, 544.0f, 111.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(138.0f, 618.0f, 111.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;

	ItemSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint2"));
	ItemSpawnPoint2->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint2->SetRelativeLocation(FVector(1178.0f, 730.0f, 111.0f));
	ItemSpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint2->bHiddenInGame = true;

	ItemSpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint3"));
	ItemSpawnPoint3->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint3->SetRelativeLocation(FVector(1178.0f, 824.0f, 111.0f));
	ItemSpawnPoint3->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint3->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint3->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(670.0f, 840.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(370.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(641.5f, 600.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.75f, 29.25f, 10.5f));
	
	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(643.5f, 560.0f, -16.7f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
