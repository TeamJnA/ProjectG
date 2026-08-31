// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGGhostRoom.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Level/Misc/PGPhotoSpot.h"
#include "Components/BoxComponent.h"


APGGhostRoom::APGGhostRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionGhostRoom.LI_MansionGhostRoom_C"));

	WallClass = APGWall_Brown::StaticClass();

	OverlapBox->SetRelativeLocation(FVector(610.0f, -310.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(18.4f, 18.5f, 10.4f));

	EnemySpawnPoint->SetRelativeLocation(FVector(495.0f, -105.0f, 200.0f));

	RoomDir->SetRelativeLocation(FVector(89.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	FuseBoxSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("FuseBoxSpawnPoint0"));
	FuseBoxSpawnPoint0->SetupAttachment(FuseBoxSpawnPointsFolder);
	FuseBoxSpawnPoint0->SetRelativeLocation(FVector(666.0f, 207.0f, 240.0f));
	FuseBoxSpawnPoint0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	FuseBoxSpawnPoint0->SetArrowColor(FLinearColor(1.0f, 1.0f, 0.0f, 0.0f));
	FuseBoxSpawnPoint0->bHiddenInGame = true;

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	RoomMesh->SetRelativeLocation(FVector(580.4f, -306.0f, -10.6f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}

	PhotoSpotConfig = { PhotoID::Room_Ghost, 10, FVector(485.0f, -562.0f, 117.0f), FRotator::ZeroRotator, FVector(60.0f, 60.0f, 80.0f) };
}

void APGGhostRoom::SpawnPhotoSpots()
{
	if (!HasAuthority())
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(PhotoSpotConfig.Offset);
	FRotator SpawnRotation = GetActorRotation() + PhotoSpotConfig.Rotation;

	FActorSpawnParameters Params;
	Params.Owner = this;

	APGPhotoSpot* Spot = GetWorld()->SpawnActor<APGPhotoSpot>(SpawnLocation, SpawnRotation, Params);
	if (Spot)
	{
		Spot->SetPhotoInfo(PhotoSpotConfig.PhotoID, PhotoSpotConfig.PhotoScore, PhotoSpotConfig.Rotation, PhotoSpotConfig.BoxExtent);
	}
}
