// Fill out your copyright notice in the Description page of Project Settings.


#include "PGStartRoom.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

APGStartRoom::APGStartRoom()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/BasicAssets/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("/Script/Engine.Material'/Game/BasicAssets/M_Basic_Floor.M_Basic_Floor'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1750.0f, -1750.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(3500.0f, 0.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = false;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(1750.0f, 1750.0f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = false;

	RoomDir->SetRelativeLocation(FVector(1450.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1750.0f, 0.0f, 0.0f));
	OverlapBox->SetRelativeScale3D(FVector(53.4f, 53.4f, 1.0f));

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh0"));
	Mesh0->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh0->SetMaterial(0, MaterialRef.Object);
	}
	Mesh0->SetRelativeLocation(FVector(1750.0f, 0.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(34.95f, 34.95f, 1.0f));

	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh1"));
	Mesh1->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh1->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh1->SetMaterial(0, MaterialRef.Object);
	}
	Mesh1->SetRelativeLocation(FVector(40.0f, 0.0f, 100.0f));
	Mesh1->SetRelativeScale3D(FVector(0.75f, 34.95f, 1.0f));

	Mesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh2"));
	Mesh2->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh2->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh2->SetMaterial(0, MaterialRef.Object);
	}
	Mesh2->SetRelativeLocation(FVector(720.0f, -1710.0f, 100.0f));
	Mesh2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh2->SetRelativeScale3D(FVector(0.75f, 14.35f, 1.0f));

	Mesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh3"));
	Mesh3->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh3->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh3->SetMaterial(0, MaterialRef.Object);
	}
	Mesh3->SetRelativeLocation(FVector(2780.0f, -1710.0f, 100.0f));
	Mesh3->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh3->SetRelativeScale3D(FVector(0.75f, 14.35f, 1.0f));

	Mesh4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh4"));
	Mesh4->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh4->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh4->SetMaterial(0, MaterialRef.Object);
	}
	Mesh4->SetRelativeLocation(FVector(3460.0f, -1030.0f, 100.0f));
	Mesh4->SetRelativeScale3D(FVector(0.75f, 14.35f, 1.0f));

	Mesh5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh5"));
	Mesh5->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh5->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh5->SetMaterial(0, MaterialRef.Object);
	}
	Mesh5->SetRelativeLocation(FVector(3460.0f, 1030.0f, 100.0f));
	Mesh5->SetRelativeScale3D(FVector(0.75f, 14.35f, 1.0f));

	Mesh6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh6"));
	Mesh6->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh6->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh6->SetMaterial(0, MaterialRef.Object);
	}
	Mesh6->SetRelativeLocation(FVector(2780.f, 1710.f, 100.0f));
	Mesh6->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh6->SetRelativeScale3D(FVector(0.75f, 14.35f, 1.0f));

	Mesh7 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh7"));
	Mesh7->SetupAttachment(GeometryFolder);
	if (MeshRef.Object)
	{
		Mesh7->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh7->SetMaterial(0, MaterialRef.Object);
	}
	Mesh7->SetRelativeLocation(FVector(720.0f, 1710.0f, 100.0f));
	Mesh7->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh7->SetRelativeScale3D(FVector(0.75f, 14.35f, 1.0f));
}
