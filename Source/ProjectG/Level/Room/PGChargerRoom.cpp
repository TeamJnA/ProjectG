// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGChargerRoom.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGChargerRoom::APGChargerRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionChargerRoom.LI_MansionChargerRoom_C"));

	WallClass = APGWall::StaticClass();

	OverlapBox->SetRelativeLocation(FVector(634.0f, 295.0f, 480.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.25f, 19.5f, 14.75f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(1255.0f, -940.0f, 695.0f));
	OverlapBox1->SetRelativeScale3D(FVector(19.25f, 18.5f, 8.25f));

	EnemySpawnPoint->SetRelativeLocation(FVector(1090.0f, -920.0f, 630.0f));

	RoomDir->SetRelativeLocation(FVector(89.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(946.5f, -300.1f, -25.1f));
}
