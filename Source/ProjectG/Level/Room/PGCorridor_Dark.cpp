// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGCorridor_Dark.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"

APGCorridor_Dark::APGCorridor_Dark()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionCorridor_Dark.LI_MansionCorridor_Dark_C"));

	WallClass = APGWall_Brown::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1571.95f, -411.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
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
	ExitDir3->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = true;

	ExitDir4 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection4"));
	ExitDir4->SetupAttachment(ExitPointsFolder);
	ExitDir4->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir4->SetRelativeLocation(FVector(971.95f, 407.0f, 0.0f));
	ExitDir4->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir4->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir4->bHiddenInGame = true;

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(1272.0f, 312.0f, 104.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(1872.0f, -316.0f, 104.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;
	
	ItemSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint2"));
	ItemSpawnPoint2->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint2->SetRelativeLocation(FVector(2473.0f, 312.0f, 104.0f));
	ItemSpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint2->bHiddenInGame = true;

	ItemSpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint3"));
	ItemSpawnPoint3->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint3->SetRelativeLocation(FVector(3072.0f, -316.0f, 104.0f));
	ItemSpawnPoint3->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint3->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint3->bHiddenInGame = true;

	MannequinSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint0"));
	MannequinSpawnPoint0->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint0->SetRelativeLocation(FVector(780.0f, 280.0f, 6.0f));
	MannequinSpawnPoint0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MannequinSpawnPoint0->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint0->bHiddenInGame = true;

	MannequinSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint1"));
	MannequinSpawnPoint1->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint1->SetRelativeLocation(FVector(1380.0f, -300.0f, 6.0f));
	MannequinSpawnPoint1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	MannequinSpawnPoint1->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint2"));
	MannequinSpawnPoint2->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint2->SetRelativeLocation(FVector(1980.0f, 300.0f, 6.0f));
	MannequinSpawnPoint2->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MannequinSpawnPoint2->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint2->bHiddenInGame = true;

	MannequinSpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint3"));
	MannequinSpawnPoint3->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint3->SetRelativeLocation(FVector(2580.0f, -300.0f, 6.0f));
	MannequinSpawnPoint3->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	MannequinSpawnPoint3->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint3->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(1840.0f, 0.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1830.0f, -5.0f, 310.0f));
	OverlapBox->SetRelativeScale3D(FVector(56.0f, 12.0f, 9.25f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(1868.0f, 0.0f, -30.0f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
