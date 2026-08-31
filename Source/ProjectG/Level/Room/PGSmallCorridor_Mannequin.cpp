// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGSmallCorridor_Mannequin.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGSmallCorridor_Mannequin::APGSmallCorridor_Mannequin()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionSmallCorridor_Mannequin.LI_MansionSmallCorridor_Mannequin_C"));

	WallClass = APGWall::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1800.0f, -10.1f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(920.0f, 0.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(900.0f, -295.0f, 275.0f));
	OverlapBox->SetRelativeScale3D(FVector(27.25f, 19.0f, 8.25f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(923.0f, -286.94f, -18.2f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}
