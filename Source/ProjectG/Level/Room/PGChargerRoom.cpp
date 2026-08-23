// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGChargerRoom.h"
#include "Level/Misc/PGWall.h"
#include "Level/Misc/PGPhotoSpot.h"
#include "Components/BoxComponent.h"


APGChargerRoom::APGChargerRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionMannequinRoom.LI_MansionMannequinRoom_C"));

	WallClass = APGWall::StaticClass();

	OverlapBox->SetRelativeLocation(FVector(1450.0f, 240.0f, 0.0f));
	OverlapBox->SetRelativeScale3D(FVector(44.5f, 20.1f, 12.75f));

	EnemySpawnPoint->SetRelativeLocation(FVector(2200.0f, 390.0f, -180.0f));

	RoomDir->SetRelativeLocation(FVector(89.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(1430.0f, 240.6f, -446.0f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	PhotoSpotConfigs = {
		{ PhotoID::Room_Charger, 10, FVector(1595.0f, 390.0f, -340.0f) },
		{ PhotoID::Room_Charger, 10, FVector(2130.0f, 730.0f, -250.0f), FRotator::ZeroRotator, FVector(128.0f, 32.0f, 32.0f) },
		{ PhotoID::Room_Charger, 10, FVector(2475.0f, 590.0f, -195.0f), FRotator(0.0f, 22.5f, 0.0f), FVector(32.0f, 128.0f, 64.0f) },
		{ PhotoID::Room_Charger, 10, FVector(2410.0f, -200.0f, -250.0f), FRotator::ZeroRotator, FVector(160.0f, 32.0f, 64.0f) }
	};
}

void APGChargerRoom::SpawnPhotoSpots()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const FPhotoSpotConfig& Config : PhotoSpotConfigs)
	{
		FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(Config.Offset);
		FRotator SpawnRotation = GetActorRotation() + Config.Rotation;

		FActorSpawnParameters Params;
		Params.Owner = this;

		APGPhotoSpot* Spot = GetWorld()->SpawnActor<APGPhotoSpot>(SpawnLocation, SpawnRotation, Params);
		if (Spot)
		{
			Spot->SetPhotoInfo(Config.PhotoID, Config.PhotoScore, Config.Rotation, Config.BoxExtent);
		}
	}
}
