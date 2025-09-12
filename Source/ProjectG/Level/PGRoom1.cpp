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
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1571.95f, -411.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(2771.95f, -411.0f, 0.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = true;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitPointsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(3691.0f, 0.0f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = true;

	ExitDir3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection3"));
	ExitDir3->SetupAttachment(ExitPointsFolder);
	ExitDir3->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir3->SetRelativeLocation(FVector(2171.95f, 407.0f, 0.0f));
	ExitDir3->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = true;

	ExitDir4 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection4"));
	ExitDir4->SetupAttachment(ExitPointsFolder);
	ExitDir4->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir4->SetRelativeLocation(FVector(971.95f, 407.0f, 0.0f));
	ExitDir4->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir4->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir4->bHiddenInGame = true;

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(594.0f, -344.0f, 105.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(1840.0f, 0.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1830.0f, -5.0f, 310.0f));
	OverlapBox->SetRelativeScale3D(FVector(56.75f, 12.0f, 9.25f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1885.0f, 63.0f, -49.1f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
