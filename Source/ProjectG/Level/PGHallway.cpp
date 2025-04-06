// Fill out your copyright notice in the Description page of Project Settings.


#include "PGHallway.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

APGHallway::APGHallway()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/BasicAssets/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("/Script/Engine.Material'/Game/BasicAssets/M_Basic_Floor.M_Basic_Floor'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1980.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

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
	Mesh0->SetRelativeLocation(FVector(990.0f, 0.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(19.75f, 7.75f, 1.0f));

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
	Mesh1->SetRelativeLocation(FVector(990.0f, -350.0f, 100.0f));
	Mesh1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh1->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));

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
	Mesh2->SetRelativeLocation(FVector(990.0f, 350.0f, 100.0f));
	Mesh2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh2->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));
}
