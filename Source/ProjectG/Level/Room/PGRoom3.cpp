// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGRoom3.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"

APGRoom3::APGRoom3()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionBedroom.LI_MansionBedroom_C'"));

	WallClass = APGWall_Brown::StaticClass();

	FuseBoxSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("FuseBoxSpawnPoint0"));
	FuseBoxSpawnPoint0->SetupAttachment(FuseBoxSpawnPointsFolder);
	FuseBoxSpawnPoint0->SetRelativeLocation(FVector(1200.0f, 715.0f, 200.0f));
	FuseBoxSpawnPoint0->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	FuseBoxSpawnPoint0->SetArrowColor(FLinearColor(1.0f, 1.0f, 0.0f, 0.0f));
	FuseBoxSpawnPoint0->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(670.0f, 840.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(370.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(620.0f, 546.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.0f, 30.25f, 10.5f));
	
	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(604.5f, 549.9f, -16.7f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
