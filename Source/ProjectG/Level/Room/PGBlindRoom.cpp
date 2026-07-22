// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGBlindRoom.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Level/Misc/PGPhotoSpot.h"
#include "Components/BoxComponent.h"
#include "Gimmick/TriggerGimmick/PGSwingProp.h"


APGBlindRoom::APGBlindRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionBlindRoom.LI_MansionBlindRoom_C"));

	WallClass = APGWall_Brown::StaticClass();

	SwingPropPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("SwingPropPointsFolder"));
	SwingPropPointsFolder->SetupAttachment(RootComponent);

	OverlapBox->SetRelativeLocation(FVector(610.0f, -605.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(18.3f, 28.7f, 10.4f));

	EnemySpawnPoint->SetRelativeLocation(FVector(375.0f, -167.0f, 200.0f));

	RoomDir->SetRelativeLocation(FVector(89.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	RoomMesh->SetRelativeLocation(FVector(585.4f, -602.1f, -29.5f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}

	PhotoSpotConfigs = {
		{ PhotoID::Room_Blind, 10, FVector(375.0f, -630.0f, 164.0f), FRotator::ZeroRotator, FVector(115.0f, 32.0f, 32.0f) },
		{ PhotoID::Room_Blind, 10, FVector(749.0f, -1268.0f, 146.0f) },
		{ PhotoID::Room_Blind, 10, FVector(415.0f, -854.0f, 218.0f), FRotator::ZeroRotator, FVector(64.0f, 64.0f, 64.0f) },
		{ PhotoID::Room_Blind, 10, FVector(415.0f, -1062.0f, 218.0f), FRotator::ZeroRotator, FVector(64.0f, 64.0f, 64.0f) },
		{ PhotoID::Room_Blind, 10, FVector(415.0f, -1278.0f, 218.0f), FRotator::ZeroRotator, FVector(64.0f, 64.0f, 64.0f) }
	};
}

void APGBlindRoom::SpawnPhotoSpots()
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

void APGBlindRoom::SpawnSwingProps(const FRandomStream& InStream)
{
	if (!HasAuthority() || !SwingPropClass || !SwingPropPointsFolder)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<USceneComponent*> Points = SwingPropPointsFolder->GetAttachChildren();
	if (Points.IsEmpty())
	{
		return;
	}

	for (int32 i = Points.Num() - 1; i > 0; --i)
	{
		const int32 j = InStream.RandRange(0, i);
		Points.Swap(i, j);
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const int32 SpawnCount = FMath::Min(SwingPropSpawnCount, Points.Num());
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		if (!Points[i])
		{
			continue;
		}

		APGSwingProp* Prop = World->SpawnActor<APGSwingProp>(SwingPropClass, Points[i]->GetComponentTransform(), Params);
		if (Prop)
		{
			Prop->SetMaterialVariation(InStream.RandRange(0, Prop->GetMaterialVariationCount() - 1));
		}
	}
}
