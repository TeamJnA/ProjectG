// Fill out your copyright notice in the Description page of Project Settings.


#include "PGStairRoom1.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGStairRoom1::APGStairRoom1()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_MansionBigRoom.LI_MansionBigRoom_C'"));

	OverlapBox->SetRelativeLocation(FVector(645.0f, 295.0f, 480.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.8f, 19.8f, 14.75f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(1280.0f, -940.0f, 695.0f));
	OverlapBox1->SetRelativeScale3D(FVector(19.6f, 18.5f, 8.25f));

	RoomDir->SetRelativeLocation(FVector(100.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(962.5f, -300.1f, -25.1f));

}
