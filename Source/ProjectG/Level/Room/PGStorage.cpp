// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStorage.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"


APGStorage::APGStorage()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionStorage.LI_MansionStorage_C"));

	WallClass = APGWall_Brown::StaticClass();

	RoomDir->SetRelativeLocation(FVector(190.0f, 0.0f, 130.0f));
	EnemySpawnPoint->SetRelativeLocation(FVector(430.0f, -10.0f, 220.0f));
	OverlapBox->SetRelativeLocation(FVector(610.0f, 240.0f, 340.0f));
	OverlapBox->SetRelativeScale3D(FVector(18.5f, 20.0f, 10.25f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	RoomMesh->SetRelativeLocation(FVector(606.9f, 246.2f, -49.6f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
}
