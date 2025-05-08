// Fill out your copyright notice in the Description page of Project Settings.


#include "PGStairRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGStairRoom1::APGStairRoom1()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_StairRoom.LI_StairRoom_C'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->SetRelativeLocation(FVector(1989.0f, -429.0f, -635.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetArrowLength(80.1f);
	ExitDir1->SetRelativeLocation(FVector(1748.0f, 300.9f, -635.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = false;

	OverlapBox->SetRelativeLocation(FVector(995.0f, -165.0f, 0.0f));
	OverlapBox->SetRelativeScale3D(FVector(30.0f, 14.25f, 19.5f));

	RoomDir->SetRelativeLocation(FVector(100.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1069.6f, -214.6f, -691.0f));

}
