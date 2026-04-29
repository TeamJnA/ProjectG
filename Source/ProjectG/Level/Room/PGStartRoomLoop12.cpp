// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStartRoomLoop12.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGStartRoomLoop12::APGStartRoomLoop12()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionStartRoomLoop12.LI_MansionStartRoomLoop12_C"));

	WallClass = APGWall::StaticClass();

	LoopStartExit = EStartRoomExit::ExitDir1;
	LoopEndExit = EStartRoomExit::ExitDir2;

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1456.0f, -803.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(2976.0f, -26.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = true;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitPointsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(2976.0f, 1800.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = true;

	ExitDir3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection3"));
	ExitDir3->SetupAttachment(ExitPointsFolder);
	ExitDir3->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir3->SetRelativeLocation(FVector(1517.0f, 2565.9f, 0.0f));
	ExitDir3->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = true;

	RoomDir->SetRelativeLocation(FVector(300.0f, 0.0f, 130.0f));

	EnemySpawnPoint->SetRelativeLocation(FVector(1990.0f, 900.0f, 250.0f));

	OverlapBox->SetRelativeLocation(FVector(1787.0f, 882.0f, 271.0f));
	OverlapBox->SetRelativeScale3D(FVector(36.5f, 52.0f, 8.2f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(1488.0f, 882.2f, 0.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}
