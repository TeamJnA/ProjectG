// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGStartRoom.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGStartRoom::APGStartRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionMainRoom.LI_MansionMainRoom_C'"));
	static ConstructorHelpers::FClassFinder<AActor> WindowGimmickRef(TEXT("/Game/ProjectG/Gimmick/Trigger/WindowBlood/BP_PGTriggerGimmickWindowBlood.BP_PGTriggerGimmickWindowBlood_C"));


	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(945.35f, -1209.3f, 414.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = true;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitPointsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(1874.4f, 319.8f, 414.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = true;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitPointsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(1535.3f, 1218.7f, 4.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = true;

	ExitDir3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection3"));
	ExitDir3->SetupAttachment(ExitPointsFolder);
	ExitDir3->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir3->SetRelativeLocation(FVector(335.3f, 1218.7f, 4.0f));
	ExitDir3->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = true;

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(940.0f, 5.0f, 500.0f));
	OverlapBox->SetRelativeScale3D(FVector(29.0f, 37.75f, 15.5f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(2740.0f, 320.0f, -35.0f));
	OverlapBox1->SetRelativeScale3D(FVector(26.5f, 26.5f, 11.0f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(1018.0f, 0.0f, -66.1f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

	WindowGimmick0 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick0"));
	WindowGimmick0->SetupAttachment(Root);
	WindowGimmick0->SetRelativeLocation(FVector(1811.0f, -741.0f, 480.0f));
	WindowGimmick0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick1"));
	WindowGimmick1->SetupAttachment(Root);
	WindowGimmick1->SetRelativeLocation(FVector(1811.0f, -741.0f, 60.0f));
	WindowGimmick1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	WindowGimmick2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick2"));
	WindowGimmick2->SetupAttachment(Root);
	WindowGimmick2->SetRelativeLocation(FVector(1692.0f, 1155.0f, 480.0f));
	WindowGimmick2->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	WindowGimmick3 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick3"));
	WindowGimmick3->SetupAttachment(Root);
	WindowGimmick3->SetRelativeLocation(FVector(1378.5f, 1155.0f, 480.0f));
	WindowGimmick3->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	WindowGimmick4 = CreateDefaultSubobject<UChildActorComponent>(TEXT("WindowGimmick4"));
	WindowGimmick4->SetupAttachment(Root);
	WindowGimmick4->SetRelativeLocation(FVector(67.0f, 791.5f, 60.0f));
	WindowGimmick4->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

	if (WindowGimmickRef.Succeeded())
	{
		WindowGimmick0->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick1->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick2->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick3->SetChildActorClass(WindowGimmickRef.Class);
		WindowGimmick4->SetChildActorClass(WindowGimmickRef.Class);
	}
}
