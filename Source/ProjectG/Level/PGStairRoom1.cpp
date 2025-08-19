// Fill out your copyright notice in the Description page of Project Settings.


#include "PGStairRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGStairRoom1::APGStairRoom1()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_MansionBigRoom.LI_MansionBigRoom_C'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1888.0f, -1214.0f, 414.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	OverlapBox->SetRelativeLocation(FVector(634.0f, 295.0f, 480.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.5f, 19.5f, 14.75f));

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

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(951.5f, -300.1f, -25.1f));
}
