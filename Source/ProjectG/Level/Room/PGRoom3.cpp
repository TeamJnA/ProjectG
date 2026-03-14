// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGRoom3.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"

APGRoom3::APGRoom3()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionBedroom.LI_MansionBedroom_C'"));
	static ConstructorHelpers::FClassFinder<AActor> WindowGimmickRef(TEXT("/Game/ProjectG/Gimmick/Trigger/WindowBlood/BP_PGTriggerGimmickWindowBlood.BP_PGTriggerGimmickWindowBlood_C"));

	WallClass = APGWall_Brown::StaticClass();

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(611.0f, 1440.0f, 111.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(721.0f, 1440.0f, 111.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;

	ItemSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint2"));
	ItemSpawnPoint2->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint2->SetRelativeLocation(FVector(1126.0f, -95.0f, 116.0f));
	ItemSpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint2->bHiddenInGame = true;

	MannequinSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint0"));
	MannequinSpawnPoint0->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint0->SetRelativeLocation(FVector(921.0f, 570.0f, 6.0f));
	MannequinSpawnPoint0->SetRelativeRotation(FRotator(0.0f, 260.0f, 0.0f));
	MannequinSpawnPoint0->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint0->bHiddenInGame = true;

	MannequinSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint1"));
	MannequinSpawnPoint1->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint1->SetRelativeLocation(FVector(395.0f, 1360.0f, 6.0f));
	MannequinSpawnPoint1->SetRelativeRotation(FRotator(0.0f, -70.0f, 0.0f));
	MannequinSpawnPoint1->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint2"));
	MannequinSpawnPoint2->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint2->SetRelativeLocation(FVector(705.0f, -134.0f, 6.0f));
	MannequinSpawnPoint2->SetRelativeRotation(FRotator(0.0f, -50.0f, 0.0f));
	MannequinSpawnPoint2->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint2->bHiddenInGame = true;

	FuseBoxSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("FuseBoxSpawnPoint0"));
	FuseBoxSpawnPoint0->SetupAttachment(FuseBoxSpawnPointsFolder);
	FuseBoxSpawnPoint0->SetRelativeLocation(FVector(1200.0f, 715.0f, 200.0f));
	FuseBoxSpawnPoint0->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	FuseBoxSpawnPoint0->SetArrowColor(FLinearColor(1.0f, 1.0f, 0.0f, 0.0f));
	FuseBoxSpawnPoint0->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(670.0f, 840.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(370.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(620.0f, 600.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.0f, 29.25f, 10.5f));
	
	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(604.5f, 549.9f, -16.7f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	WindowGimmick0 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick0"));
	WindowGimmick0->SetupAttachment(Root);
	WindowGimmick0->SetRelativeLocation(FVector(448.5f, -288.1f, 66.0f));
	WindowGimmick0->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	WindowGimmick1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick1"));
	WindowGimmick1->SetupAttachment(Root);
	WindowGimmick1->SetRelativeLocation(FVector(1176.0f, 128.4f, 66.0f));
	WindowGimmick1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick2"));
	WindowGimmick2->SetupAttachment(Root);
	WindowGimmick2->SetRelativeLocation(FVector(1176.0f, 1042.9f, 66.0f));
	WindowGimmick2->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick3 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick3"));
	WindowGimmick3->SetupAttachment(Root);
	WindowGimmick3->SetRelativeLocation(FVector(1176.0f, 1356.4f, 66.0f));
	WindowGimmick3->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	if (WindowGimmickRef.Succeeded())
	{
		WindowGimmick0->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick1->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick2->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick3->SetChildActorClass(WindowGimmickRef.Class);
	}
}
