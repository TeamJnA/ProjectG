// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStartRoomLoop03.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGStartRoomLoop03::APGStartRoomLoop03()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionStartRoomLoop03.LI_MansionStartRoomLoop03_C"));

	WallClass = APGWall::StaticClass();

	LoopStartExit = EStartRoomExit::ExitDir3;
	LoopEndExit = EStartRoomExit::ExitDir0;

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(-223.0f, 859.8f, -1059.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(-1789.0f, 859.8f, -1059.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = true;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitPointsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(-317.0f, -1144.0f, -1059.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = true;

	ExitDir3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection3"));
	ExitDir3->SetupAttachment(ExitPointsFolder);
	ExitDir3->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir3->SetRelativeLocation(FVector(-2203.0f, -1469.0f, -1059.0f));
	ExitDir3->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = true;

	RoomDir->SetRelativeLocation(FVector(300.0f, 0.0f, 130.0f));

	EnemySpawnPoint->SetRelativeLocation(FVector(-980.0f, 610.0f, -800.0f));

	OverlapBox->SetRelativeLocation(FVector(800.0f, 326.0f, -259.0f));
	OverlapBox->SetRelativeScale3D(FVector(17.75f, 19.45f, 24.5f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(-996.0f, 599.0f, -786.0f));
	OverlapBox1->SetRelativeScale3D(FVector(33.55f, 7.9f, 8.1f));

	OverlapBox2 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox2"));
	OverlapBox2->SetupAttachment(OverlapBoxFolder);
	OverlapBox2->CanCharacterStepUpOn = ECB_No;
	OverlapBox2->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox2->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox2->SetRelativeLocation(FVector(-1210.0f, -305.0f, -786.0f));
	OverlapBox2->SetRelativeScale3D(FVector(26.85f, 19.2f, 8.1f));

	OverlapBox3 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox3"));
	OverlapBox3->SetupAttachment(OverlapBoxFolder);
	OverlapBox3->CanCharacterStepUpOn = ECB_No;
	OverlapBox3->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox3->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox3->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox3->SetRelativeLocation(FVector(-1424.0f, -1200.0f, -786.0f));
	OverlapBox3->SetRelativeScale3D(FVector(34.275f, 8.035f, 8.1f));

	OverlapBox4 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox4"));
	OverlapBox4->SetupAttachment(OverlapBoxFolder);
	OverlapBox4->CanCharacterStepUpOn = ECB_No;
	OverlapBox4->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox4->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox4->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox4->SetRelativeLocation(FVector(-3185.0f, -341.0f, -259.0f));
	OverlapBox4->SetRelativeScale3D(FVector(19.25f, 19.1825f, 24.5f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(-1043.0f, -261.8f, -1059.0f));
}
