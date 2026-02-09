// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGSmallCorridor_Mannequin.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGSmallCorridor_Mannequin::APGSmallCorridor_Mannequin()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionSmallCorridor_Mannequin.LI_MansionSmallCorridor_Mannequin_C"));
	static ConstructorHelpers::FClassFinder<AActor> WindowGimmickRef(TEXT("/Game/ProjectG/Gimmick/Trigger/WindowBlood/BP_PGTriggerGimmickWindowBlood.BP_PGTriggerGimmickWindowBlood_C"));

	WallClass = APGWall::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1800.0f, -10.1f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = true;

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(1160.0f, -420.0f, 105.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(1160.0f, -675.0f, 105.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint0"));
	MannequinSpawnPoint0->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint0->SetRelativeLocation(FVector(904.0f, -604.0f, 6.0f));
	MannequinSpawnPoint0->SetRelativeRotation(FRotator(0.0f, -130.0f, 0.0f));
	MannequinSpawnPoint0->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint0->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(920.0f, 0.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(900.0f, -295.0f, 275.0f));
	OverlapBox->SetRelativeScale3D(FVector(27.25f, 19.0f, 8.25f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(923.0f, -286.94f, -18.2f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick0 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick0"));
	WindowGimmick0->SetupAttachment(Root);
	WindowGimmick0->SetRelativeLocation(FVector(743.3f, 264.0f, 66.0f));

	WindowGimmick1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick1"));
	WindowGimmick1->SetupAttachment(Root);
	WindowGimmick1->SetRelativeLocation(FVector(1057.0f, 264.0f, 66.0f));

	if (WindowGimmickRef.Succeeded())
	{
		WindowGimmick0->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick1->SetChildActorClass(WindowGimmickRef.Class);
	}
}
