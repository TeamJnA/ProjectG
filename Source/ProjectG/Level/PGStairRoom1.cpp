// Fill out your copyright notice in the Description page of Project Settings.


#include "PGStairRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

APGStairRoom1::APGStairRoom1()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/BasicAssets/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("/Script/Engine.Material'/Game/BasicAssets/M_Basic_Floor.M_Basic_Floor'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1980.0f, 0.0f, 1075.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	OverlapBox->SetRelativeLocation(FVector(990.0f, 0.0f, 590.0f));
	OverlapBox->SetRelativeScale3D(FVector(30.0f, 30.0f, 19.0f));

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 1190.0f));
	RoomDir->SetRelativeScale3D(FVector(7.75f, 7.75f, 7.75f));

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

	Mesh2->SetRelativeLocation(FVector(990.0f, -950.0f, 100.0f));
	Mesh2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh2->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));

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

	Mesh3->SetRelativeLocation(FVector(1940.0f, 0.0f, 100.0f));
	Mesh3->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));

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

	Mesh4->SetRelativeLocation(FVector(990.0f, 950.0f, 100.0f));
	Mesh4->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh4->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));

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

	Mesh5->SetRelativeLocation(FVector(40.0f, 650.0f, 100.0f));
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

	Mesh6->SetRelativeLocation(FVector(525.0f, 0.0f, 55.0f));
	Mesh6->SetRelativeScale3D(FVector(3.85f, 6.75f, 1.0f));

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

	Mesh7->SetRelativeLocation(FVector(610.0f, 0.0f, 75.0f));
	Mesh7->SetRelativeScale3D(FVector(3.85f, 6.75f, 1.5f));

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

	Mesh8->SetRelativeLocation(FVector(695.0f, 0.0f, 110.0f));
	Mesh8->SetRelativeScale3D(FVector(3.85f, 6.75f, 2.1f));

	Mesh9 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh9"));
	if (MeshRef.Object)
	{
		Mesh9->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh9->SetMaterial(0, MaterialRef.Object);
	}
	Mesh9->SetupAttachment(GeometryFolder);

	Mesh9->SetRelativeLocation(FVector(790.0f, 0.0f, 140.0f));
	Mesh9->SetRelativeScale3D(FVector(3.85f, 6.75f, 2.7f));

	Mesh10 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh10"));
	if (MeshRef.Object)
	{
		Mesh10->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh10->SetMaterial(0, MaterialRef.Object);
	}
	Mesh10->SetupAttachment(GeometryFolder);

	Mesh10->SetRelativeLocation(FVector(895.0f, 0.0f, 160.0f));
	Mesh10->SetRelativeScale3D(FVector(3.85f, 6.75f, 3.7f));

	Mesh11 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh11"));
	if (MeshRef.Object)
	{
		Mesh11->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh11->SetMaterial(0, MaterialRef.Object);
	}
	Mesh11->SetupAttachment(GeometryFolder);

	Mesh11->SetRelativeLocation(FVector(1005.0f, 0.0f, 210.0f));
	Mesh11->SetRelativeScale3D(FVector(3.85f, 6.75f, 4.4f));

	Mesh12 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh12"));
	if (MeshRef.Object)
	{
		Mesh12->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh12->SetMaterial(0, MaterialRef.Object);
	}
	Mesh12->SetupAttachment(GeometryFolder);

	Mesh12->SetRelativeLocation(FVector(1090.0f, 0.0f, 260.0f));
	Mesh12->SetRelativeScale3D(FVector(3.85f, 6.75f, 5.0f));

	Mesh13 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh13"));
	if (MeshRef.Object)
	{
		Mesh13->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh13->SetMaterial(0, MaterialRef.Object);
	}
	Mesh13->SetupAttachment(GeometryFolder);

	Mesh13->SetRelativeLocation(FVector(1180.0f, 0.0f, 295.0f));
	Mesh13->SetRelativeScale3D(FVector(3.85f, 6.75f, 5.7f));

	Mesh14 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh14"));
	if (MeshRef.Object)
	{
		Mesh14->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh14->SetMaterial(0, MaterialRef.Object);
	}
	Mesh14->SetupAttachment(GeometryFolder);

	Mesh14->SetRelativeLocation(FVector(1280.0f, 0.0f, 340.0f));
	Mesh14->SetRelativeScale3D(FVector(3.85f, 6.75f, 6.7f));

	Mesh15 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh15"));
	if (MeshRef.Object)
	{
		Mesh15->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh15->SetMaterial(0, MaterialRef.Object);
	}
	Mesh15->SetupAttachment(GeometryFolder);

	Mesh15->SetRelativeLocation(FVector(1370.0f, 0.0f, 375.0f));
	Mesh15->SetRelativeScale3D(FVector(3.85f, 6.75f, 7.5f));

	Mesh16 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh16"));
	if (MeshRef.Object)
	{
		Mesh16->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh16->SetMaterial(0, MaterialRef.Object);
	}
	Mesh16->SetupAttachment(GeometryFolder);

	Mesh16->SetRelativeLocation(FVector(1465.0f, 0.0f, 425.0f));
	Mesh16->SetRelativeScale3D(FVector(3.85f, 6.75f, 8.1f));

	Mesh17 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh17"));
	if (MeshRef.Object)
	{
		Mesh17->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh17->SetMaterial(0, MaterialRef.Object);
	}
	Mesh17->SetupAttachment(GeometryFolder);

	Mesh17->SetRelativeLocation(FVector(1545.0f, 0.0f, 445.0f));
	Mesh17->SetRelativeScale3D(FVector(3.85f, 6.75f, 9.0f));

	Mesh18 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh18"));
	if (MeshRef.Object)
	{
		Mesh18->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh18->SetMaterial(0, MaterialRef.Object);
	}
	Mesh18->SetupAttachment(GeometryFolder);

	Mesh18->SetRelativeLocation(FVector(1635.0f, 0.0f, 495.0f));
	Mesh18->SetRelativeScale3D(FVector(3.85f, 6.75f, 9.6f));

	Mesh19 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh19"));
	if (MeshRef.Object)
	{
		Mesh19->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh19->SetMaterial(0, MaterialRef.Object);
	}
	Mesh19->SetupAttachment(GeometryFolder);

	Mesh19->SetRelativeLocation(FVector(1710.0f, 0.0f, 540.0f));
	Mesh19->SetRelativeScale3D(FVector(3.85f, 6.75f, 10.3f));

	Mesh20 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh20"));
	if (MeshRef.Object)
	{
		Mesh20->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh20->SetMaterial(0, MaterialRef.Object);
	}
	Mesh20->SetupAttachment(GeometryFolder);

	Mesh20->SetRelativeLocation(FVector(200.0f, 0.0f, 1075.0f));
	Mesh20->SetRelativeScale3D(FVector(3.95f, 19.75f, 1.0f));

	Mesh21 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh21"));
	if (MeshRef.Object)
	{
		Mesh21->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh21->SetMaterial(0, MaterialRef.Object);
	}
	Mesh21->SetupAttachment(GeometryFolder);

	Mesh21->SetRelativeLocation(FVector(990.0f, -720.0f, 1075.0f));
	Mesh21->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh21->SetRelativeScale3D(FVector(5.35f, 19.75f, 1.0f));

	Mesh22 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh22"));
	if (MeshRef.Object)
	{
		Mesh22->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh22->SetMaterial(0, MaterialRef.Object);
	}
	Mesh22->SetupAttachment(GeometryFolder);

	Mesh22->SetRelativeLocation(FVector(1780.0f, 0.0f, 1075.0f));
	Mesh22->SetRelativeScale3D(FVector(3.95f, 19.75f, 1.0f));

	Mesh23 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh23"));
	if (MeshRef.Object)
	{
		Mesh23->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh23->SetMaterial(0, MaterialRef.Object);
	}
	Mesh23->SetupAttachment(GeometryFolder);

	Mesh23->SetRelativeLocation(FVector(990.0f, 720.0f, 1075.0f));
	Mesh23->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh23->SetRelativeScale3D(FVector(5.35f, 19.75f, 1.0f));

	Mesh24 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh24"));
	if (MeshRef.Object)
	{
		Mesh24->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh24->SetMaterial(0, MaterialRef.Object);
	}
	Mesh24->SetupAttachment(GeometryFolder);

	Mesh24->SetRelativeLocation(FVector(40.0f, 0.0f, 1175.0f));
	Mesh24->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));

	Mesh25 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh25"));
	if (MeshRef.Object)
	{
		Mesh25->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh25->SetMaterial(0, MaterialRef.Object);
	}
	Mesh25->SetupAttachment(GeometryFolder);

	Mesh25->SetRelativeLocation(FVector(990.0f, -950.0f, 1175.0f));
	Mesh25->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh25->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));

	Mesh26 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh26"));
	if (MeshRef.Object)
	{
		Mesh26->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh26->SetMaterial(0, MaterialRef.Object);
	}
	Mesh26->SetupAttachment(GeometryFolder);

	Mesh26->SetRelativeLocation(FVector(1940.0f, -650.0f, 1175.0f));
	Mesh26->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh27 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh27"));
	if (MeshRef.Object)
	{
		Mesh27->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh27->SetMaterial(0, MaterialRef.Object);
	}
	Mesh27->SetupAttachment(GeometryFolder);

	Mesh27->SetRelativeLocation(FVector(1940.0f, 650.0f, 1175.0f));
	Mesh27->SetRelativeScale3D(FVector(0.75f, 6.75f, 1.0f));

	Mesh28 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh28"));
	if (MeshRef.Object)
	{
		Mesh28->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh28->SetMaterial(0, MaterialRef.Object);
	}
	Mesh28->SetupAttachment(GeometryFolder);

	Mesh28->SetRelativeLocation(FVector(990.0f, 950.0f, 1175.0f));
	Mesh28->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh28->SetRelativeScale3D(FVector(0.75f, 19.75f, 1.0f));
}
