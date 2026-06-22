// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGElevatorRoom.h"
#include "Level/Misc/PGWall.h"
#include "Level/Misc/PGPhotoSpot.h"
#include "Components/BoxComponent.h"

APGElevatorRoom::APGElevatorRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionElevatorRoom.LI_MansionElevatorRoom_C"));
	static ConstructorHelpers::FClassFinder<AActor> WindowGimmickRef(TEXT("/Game/ProjectG/Gimmick/Trigger/WindowBlood/BP_PGTriggerGimmickWindowBlood.BP_PGTriggerGimmickWindowBlood_C"));
	static ConstructorHelpers::FClassFinder<AActor> ExitElevatorRef(TEXT("/Game/ProjectG/Levels/Room/Exit/ExitElevator/BP_ExitElevator.BP_ExitElevator_C"));

	WallClass = APGWall::StaticClass();
	
	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1800.0f, -10.15f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = true;

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(901.0f, -295.0f, 275.0f));
	OverlapBox->SetRelativeScale3D(FVector(27.75f, 19.25f, 8.25f));

	OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
	OverlapBox1->SetupAttachment(OverlapBoxFolder);
	OverlapBox1->CanCharacterStepUpOn = ECB_No;
	OverlapBox1->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	OverlapBox1->SetRelativeLocation(FVector(910.0f, -1250.0f, -363.0f));
	OverlapBox1->SetRelativeScale3D(FVector(12.75f, 10.25f, 24.6f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(923.0f, -614.4f, -819.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	EnemySpawnPoint->SetRelativeLocation(FVector(920.0f, 0.0f, 250.0f));

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

	ExitElevator = CreateDefaultSubobject<UChildActorComponent>(TEXT("ExitElevator"));
	ExitElevator->SetupAttachment(Root);
	if (ExitElevatorRef.Succeeded())
	{
		ExitElevator->SetChildActorClass(ExitElevatorRef.Class);
	}
	ExitElevator->SetRelativeLocation(FVector(906.0f, -1185.0f, 135.0f));
	ExitElevator->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	PhotoSpotConfig = { PhotoID::Room_Elevator, 10, FVector(910.0f, -1180.0f, 170.0f) };
}

void APGElevatorRoom::SpawnPhotoSpots()
{
	if (!HasAuthority())
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(PhotoSpotConfig.Offset);
	FRotator SpawnRotation = GetActorRotation() + PhotoSpotConfig.Rotation;

	FActorSpawnParameters Params;
	Params.Owner = this;

	APGPhotoSpot* Spot = GetWorld()->SpawnActor<APGPhotoSpot>(SpawnLocation, SpawnRotation, Params);
	if (Spot)
	{
		Spot->SetPhotoInfo(PhotoSpotConfig.PhotoID, PhotoSpotConfig.PhotoScore, PhotoSpotConfig.Rotation, PhotoSpotConfig.BoxExtent);
	}
}
