// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"

APGRoom1::APGRoom1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/BasicAssets/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("/Script/Engine.Material'/Game/BasicAssets/M_Basic_Floor.M_Basic_Floor'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(990.0f, -988.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(1978.0f, 0.0f, 0.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = false;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(990.0f, 988.0f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = false;

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh0"));
	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh0->SetMaterial(0, MaterialRef.Object);
	}
	Mesh0->SetupAttachment(GeometryFolder);

	Mesh0->SetRelativeLocation(FVector(990.0f, 0.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(19.75f, 19.75f, 1.0f));

	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh1"));
	if (MeshRef.Object)
	{
		Mesh1->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh1->SetMaterial(0, MaterialRef.Object);
	}
	Mesh1->SetupAttachment(GeometryFolder);

	Mesh1->SetRelativeLocation(FVector(40.0f, -650.0f, 100.0f));
	Mesh1->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh2"));
	if (MeshRef.Object)
	{
		Mesh2->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh2->SetMaterial(0, MaterialRef.Object);
	}
	Mesh2->SetupAttachment(GeometryFolder);

	Mesh2->SetRelativeLocation(FVector(340.0f, -950.0f, 100.0f));
	Mesh2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh2->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh3"));
	if (MeshRef.Object)
	{
		Mesh3->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh3->SetMaterial(0, MaterialRef.Object);
	}
	Mesh3->SetupAttachment(GeometryFolder);

	Mesh3->SetRelativeLocation(FVector(1640.0f, -950.0f, 100.0f));
	Mesh3->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh3->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh4"));
	if (MeshRef.Object)
	{
		Mesh4->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh4->SetMaterial(0, MaterialRef.Object);
	}
	Mesh4->SetupAttachment(GeometryFolder);

	Mesh4->SetRelativeLocation(FVector(1940.0f, -650.0f, 100.0f));
	Mesh4->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh5"));
	if (MeshRef.Object)
	{
		Mesh5->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh5->SetMaterial(0, MaterialRef.Object);
	}
	Mesh5->SetupAttachment(GeometryFolder);

	Mesh5->SetRelativeLocation(FVector(1940.0f, 650.0f, 100.0f));
	Mesh5->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh6"));
	if (MeshRef.Object)
	{
		Mesh6->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh6->SetMaterial(0, MaterialRef.Object);
	}
	Mesh6->SetupAttachment(GeometryFolder);

	Mesh6->SetRelativeLocation(FVector(1640.0f, 950.0f, 100.0f));
	Mesh6->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh6->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh7 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh7"));
	if (MeshRef.Object)
	{
		Mesh7->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh7->SetMaterial(0, MaterialRef.Object);
	}
	Mesh7->SetupAttachment(GeometryFolder);

	Mesh7->SetRelativeLocation(FVector(340.0f, 950.0f, 100.0f));
	Mesh7->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh7->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh8 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh8"));
	if (MeshRef.Object)
	{
		Mesh8->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh8->SetMaterial(0, MaterialRef.Object);
	}
	Mesh8->SetupAttachment(GeometryFolder);

	Mesh8->SetRelativeLocation(FVector(40.0f, 650.0f, 100.0f));
	Mesh8->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));
}
