// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStairRoom_Simple.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGStairRoom_Simple::APGStairRoom_Simple()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionStairRoom_Simple.LI_MansionStairRoom_Simple_C"));

	WallClass = APGWall::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(340.2f, 923.9f, 414.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(709.0f, 830.0f, 76.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(317.0f, 830.0f, 76.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint0"));
	MannequinSpawnPoint0->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint0->SetRelativeLocation(FVector(400.0f, 390.0f, 420.0f));
	MannequinSpawnPoint0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MannequinSpawnPoint0->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint0->bHiddenInGame = true;

	OverlapBox->SetRelativeLocation(FVector(634.0f, 295.0f, 480.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.3f, 19.1f, 14.75f));

	EnemySpawnPoint->SetRelativeLocation(FVector(350.0f, 40.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(89.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(627.0f, 294.8f, -25.1f));
}
