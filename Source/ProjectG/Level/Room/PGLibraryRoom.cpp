// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGLibraryRoom.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGLibraryRoom::APGLibraryRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionLibraryRoom.LI_MansionLibraryRoom_C"));
	
	WallClass = APGWall::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1547.4f, 320.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = true;	
	
	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(950.4f, -918.0f, 0.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->SetArrowLength(80.1f);
	ExitDir1->bHiddenInGame = true;

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(604.0f, -303.0f, 72.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(1304.0f, -303.0f, 72.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint0"));
	MannequinSpawnPoint0->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint0->SetRelativeLocation(FVector(670.0f, 60.0f, 6.0f));
	MannequinSpawnPoint0->SetRelativeRotation(FRotator(0.0f, 50.0f, 0.0f));
	MannequinSpawnPoint0->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint0->bHiddenInGame = true;

	MannequinSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint1"));
	MannequinSpawnPoint1->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint1->SetRelativeLocation(FVector(1640.0f, -510.0f, 6.0f));
	MannequinSpawnPoint1->SetRelativeRotation(FRotator(0.0f, -30.0f, 0.0f));
	MannequinSpawnPoint1->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint2"));
	MannequinSpawnPoint2->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint2->SetRelativeLocation(FVector(300.0f, -690.0f, 6.0f));
	MannequinSpawnPoint2->SetRelativeRotation(FRotator(0.0f, -160.0f, 0.0f));
	MannequinSpawnPoint2->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint2->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(800.0f, -650.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(940.0f, -300.0f, 340.0f));
	OverlapBox->SetRelativeScale3D(FVector(28.5f, 19.0f, 10.2f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(894.0f, -299.06f, 0.0f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}
