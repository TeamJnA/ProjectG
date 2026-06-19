// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStartRoomLoop34.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"


APGStartRoomLoop34::APGStartRoomLoop34()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionStartRoomLoop34.LI_MansionStartRoomLoop34_C"));

	WallClass = APGWall::StaticClass();

	LoopStartExit = EStartRoomExit::ExitDir4;
	LoopEndExit = EStartRoomExit::ExitDir3;

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1200.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(878.9f, -2717.0f, 421.1f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = true;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitPointsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(-1521.0f, -1518.12f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = true;

	RoomDir->SetRelativeLocation(FVector(300.0f, 0.0f, 130.0f));

	EnemySpawnPoint->SetRelativeLocation(FVector(-50.0f, -1160.0f, 240.0f));

	OverlapBox->SetRelativeLocation(FVector(600.0f, -1200.0f, 505.0f));
	OverlapBox->SetRelativeScale3D(FVector(18.35f, 47.0f, 15.5f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(-915.0f, -910.0f, 290.0f));
	OverlapBox1->SetRelativeScale3D(FVector(28.0f, 18.75f, 8.75f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(-320.9f, -948.6f, -11.9f));
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}