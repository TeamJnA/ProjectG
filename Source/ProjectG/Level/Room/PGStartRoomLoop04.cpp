// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStartRoomLoop04.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"


APGStartRoomLoop04::APGStartRoomLoop04()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionStartRoomLoop04.LI_MansionStartRoomLoop04_C"));

	WallClass = APGWall_Brown::StaticClass();

	LoopStartExit = EStartRoomExit::ExitDir4;
	LoopEndExit = EStartRoomExit::ExitDir0;

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(2408.0f, 1863.0f, -419.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(2408.0f, 3594.0f, -419.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = true;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitPointsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(591.1f, 4495.0f, -419.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = true;

	RoomDir->SetRelativeLocation(FVector(300.0f, 0.0f, 130.0f));

	EnemySpawnPoint->SetRelativeLocation(FVector(913.0f, 3124.0f, -180.0f));

	OverlapBox->SetRelativeLocation(FVector(626.0f, 596.0f, 83.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.0f, 27.9f, 15.5f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(1267.0f, 3018.0f, -146.0f));
	OverlapBox1->SetRelativeScale3D(FVector(35.15f, 45.9f, 8.0f));

	OverlapBox2 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox2"));
	OverlapBox2->SetupAttachment(OverlapBoxFolder);
	OverlapBox2->CanCharacterStepUpOn = ECB_No;
	OverlapBox2->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox2->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox2->SetRelativeLocation(FVector(-884.0f, 2773.0f, 83.0f));
	OverlapBox2->SetRelativeScale3D(FVector(30.25f, 18.75f, 15.5f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(273.9f, 2097.5f, -488.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}
