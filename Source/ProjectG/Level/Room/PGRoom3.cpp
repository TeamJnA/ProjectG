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
	ItemSpawnPoint0->SetRelativeLocation(FVector(650.0f, 1450.0f, 111.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

	ItemSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint1"));
	ItemSpawnPoint1->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint1->SetRelativeLocation(FVector(760.0f, 1450.0f, 111.0f));
	ItemSpawnPoint1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint1->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint1->bHiddenInGame = true;

	ItemSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint2"));
	ItemSpawnPoint2->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint2->SetRelativeLocation(FVector(1178.0f, 730.0f, 111.0f));
	ItemSpawnPoint2->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint2->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint2->bHiddenInGame = true;

	ItemSpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint3"));
	ItemSpawnPoint3->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint3->SetRelativeLocation(FVector(1178.0f, 824.0f, 111.0f));
	ItemSpawnPoint3->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint3->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint3->bHiddenInGame = true;

	ItemSpawnPoint4 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint4"));
	ItemSpawnPoint4->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint4->SetRelativeLocation(FVector(1165.0f, -84.0f, 116.0f));
	ItemSpawnPoint4->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint4->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint4->bHiddenInGame = true;

	MannequinSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint0"));
	MannequinSpawnPoint0->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint0->SetRelativeLocation(FVector(960.0f, 580.0f, 6.0f));
	MannequinSpawnPoint0->SetRelativeRotation(FRotator(0.0f, 260.0f, 0.0f));
	MannequinSpawnPoint0->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint0->bHiddenInGame = true;

	MannequinSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint1"));
	MannequinSpawnPoint1->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint1->SetRelativeLocation(FVector(434.0f, 1369.0f, 6.0f));
	MannequinSpawnPoint1->SetRelativeRotation(FRotator(0.0f, -70.0f, 0.0f));
	MannequinSpawnPoint1->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint1->bHiddenInGame = true;

	MannequinSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("MannequinSpawnPoint2"));
	MannequinSpawnPoint2->SetupAttachment(MannequinSpawnPointsFolder);
	MannequinSpawnPoint2->SetRelativeLocation(FVector(754.0f, -134.0f, 6.0f));
	MannequinSpawnPoint2->SetRelativeRotation(FRotator(0.0f, -50.0f, 0.0f));
	MannequinSpawnPoint2->SetArrowColor(FLinearColor(0.0f, 1.0f, 1.0f, 0.0f));
	MannequinSpawnPoint2->bHiddenInGame = true;

	EnemySpawnPoint->SetRelativeLocation(FVector(670.0f, 840.0f, 250.0f));

	RoomDir->SetRelativeLocation(FVector(370.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(641.5f, 600.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.75f, 29.25f, 10.5f));
	
	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(643.5f, 560.0f, -16.7f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	WindowGimmick0 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick0"));
	WindowGimmick0->SetupAttachment(Root);
	WindowGimmick0->SetRelativeLocation(FVector(487.5f, -278.0f, 66.0f));
	WindowGimmick0->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	WindowGimmick1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick1"));
	WindowGimmick1->SetupAttachment(Root);
	WindowGimmick1->SetRelativeLocation(FVector(1215.0f, 138.5f, 66.0f));
	WindowGimmick1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick2"));
	WindowGimmick2->SetupAttachment(Root);
	WindowGimmick2->SetRelativeLocation(FVector(1215.0f, 1053.0f, 66.0f));
	WindowGimmick2->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick3 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick3"));
	WindowGimmick3->SetupAttachment(Root);
	WindowGimmick3->SetRelativeLocation(FVector(1215.0f, 1366.5f, 66.0f));
	WindowGimmick3->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	if (WindowGimmickRef.Succeeded())
	{
		WindowGimmick0->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick1->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick2->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick3->SetChildActorClass(WindowGimmickRef.Class);
	}
}
