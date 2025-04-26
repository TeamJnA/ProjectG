// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGRoom1::APGRoom1()
{
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ProjectG/LevelAssets/Room1.Room1'"));
	// static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("/Script/Engine.Material'/Game/BasicAssets/M_Basic_Floor.M_Basic_Floor'"));
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/BP/LI_Room1Sample.LI_Room1Sample_C'"));

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(510.0f, -2040.0f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 270.f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->bHiddenInGame = false;

	ExitDir1 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection1"));
	ExitDir1->SetupAttachment(ExitsFolder);
	ExitDir1->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir1->SetRelativeLocation(FVector(2490.0f, -1540.0f, 0.0f));
	ExitDir1->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));
	ExitDir1->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir1->bHiddenInGame = false;

	ExitDir2 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection2"));
	ExitDir2->SetupAttachment(ExitsFolder);
	ExitDir2->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir2->SetRelativeLocation(FVector(2490.0f, 1100.0f, 0.0f));
	ExitDir2->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir2->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir2->bHiddenInGame = false;

	ExitDir3 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection3"));
	ExitDir3->SetupAttachment(ExitsFolder);
	ExitDir3->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir3->SetRelativeLocation(FVector(510.0f, 1540.0f, 0.0f));
	ExitDir3->SetRelativeRotation(FRotator(0.0f, 90.f, 0.0f));
	ExitDir3->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir3->bHiddenInGame = false;

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(500.0f, -250.0f, 220.0f));
	OverlapBox->SetRelativeScale3D(FVector(14.25f, 54.75f, 6.5f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(1530.0f, 0.0f, 220.0f));
	OverlapBox1->SetRelativeScale3D(FVector(16.75f, 12.5f, 6.5f));

	OverlapBox2 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox2"));
	OverlapBox2->SetupAttachment(OverlapBoxFolder);
	OverlapBox2->CanCharacterStepUpOn = ECB_No;
	OverlapBox2->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox2->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox2->SetRelativeLocation(FVector(2485.0f, -220.0f, 220.0f));
	OverlapBox2->SetRelativeScale3D(FVector(12.0f, 40.0f, 6.5f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1445.0f, -250.0f, -4.5f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
