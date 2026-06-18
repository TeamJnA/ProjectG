// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGDiningRoom.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"


APGDiningRoom::APGDiningRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionDiningRoom.LI_MansionDiningRoom_C"));

	WallClass = APGWall_Brown::StaticClass();

	RoomDir->SetRelativeLocation(FVector(290.0f, 0.0f, 130.0f));
	EnemySpawnPoint->SetRelativeLocation(FVector(400.0f, 800.0f, 220.0f));
	OverlapBox->SetRelativeLocation(FVector(630.0f, 540.0f, 340.0f));
	OverlapBox->SetRelativeScale3D(FVector(18.75f, 30.25f, 10.4f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	RoomMesh->SetRelativeLocation(FVector(607.9f, 549.9f, -7.6f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
}
