// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGRoom2.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGRoom2::APGRoom2()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionSmallCorridor.LI_MansionSmallCorridor_C'"));

	WallClass = APGWall::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1800.0f, -10.15f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = true;

	FuseBoxSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("FuseBoxSpawnPoint0"));
	FuseBoxSpawnPoint0->SetupAttachment(FuseBoxSpawnPointsFolder);
	FuseBoxSpawnPoint0->SetRelativeLocation(FVector(900.0f, -320.0f, 200.0f));
	FuseBoxSpawnPoint0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	FuseBoxSpawnPoint0->SetArrowColor(FLinearColor(1.0f, 1.0f, 0.0f, 0.0f));
	FuseBoxSpawnPoint0->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(920.0f, 0.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(901.0f, -295.0f, 275.0f));
	OverlapBox->SetRelativeScale3D(FVector(27.75f, 19.25f, 8.25f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(923.0f, -286.94f, 0.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}
