// Fill out your copyright notice in the Description page of Project Settings.


#include "PGMasterRoom.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
APGMasterRoom::APGMasterRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	GeometryFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GeometryFolder"));
	GeometryFolder->SetupAttachment(RootComponent);

	RoomDir = CreateDefaultSubobject<UArrowComponent>(TEXT("RoomDirection"));
	RoomDir->SetupAttachment(RootComponent);

	OverlapBoxFolder = CreateDefaultSubobject<USceneComponent>(TEXT("OverlapBoxFolder"));
	OverlapBoxFolder->SetupAttachment(RootComponent);

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(OverlapBoxFolder);

	ExitsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("ExitsFolder"));
	ExitsFolder->SetupAttachment(RootComponent);

	FloorSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("FloorSpawnPointsFolder"));
	FloorSpawnPointsFolder->SetupAttachment(RootComponent);

	Root->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Root->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Root->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(7.75f, 7.75f, 7.75f));

	OverlapBox->CanCharacterStepUpOn = ECB_No;
	OverlapBox->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

	OverlapBox->SetRelativeLocation(FVector(990.0f, 0.0f, 0.0f));
	OverlapBox->SetRelativeScale3D(FVector(30.0f, 30.0f, 1.0f));
}

USceneComponent* APGMasterRoom::GetExitsFolder()
{
	return ExitsFolder;
}

USceneComponent* APGMasterRoom::GetOverlapBoxFolder()
{
	return OverlapBoxFolder;
}

USceneComponent* APGMasterRoom::GetFloorSpawnPointsFolder()
{
	return FloorSpawnPointsFolder;
}
