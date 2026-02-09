// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGSmallCorridor_Bonfire.h"
#include "Level/Misc/PGWall_Brown.h"
#include "Components/BoxComponent.h"

APGSmallCorridor_Bonfire::APGSmallCorridor_Bonfire()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionSmallCorridor_Bonfire.LI_MansionSmallCorridor_Bonfire_C"));
	static ConstructorHelpers::FClassFinder<AActor> BonfireRef(TEXT("/Game/ProjectG/Gimmick/Interact/Bonfire/BP_PGInteractableGimmickBonfire.BP_PGInteractableGimmickBonfire_C"));

	WallClass = APGWall_Brown::StaticClass();

	ExitDir0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitDirection0"));
	ExitDir0->SetupAttachment(ExitPointsFolder);
	ExitDir0->SetArrowColor(FLinearColor(0.2f, 1.0f, 0.0f, 1.0f));
	ExitDir0->SetRelativeLocation(FVector(1800.0f, -10.15f, 0.0f));
	ExitDir0->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ExitDir0->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	ExitDir0->SetArrowLength(80.1f);
	ExitDir0->bHiddenInGame = true;

	ItemSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint0"));
	ItemSpawnPoint0->SetupAttachment(ItemSpawnPointsFolder);
	ItemSpawnPoint0->SetRelativeLocation(FVector(900.0f, 260.0f, 72.0f));
	ItemSpawnPoint0->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ItemSpawnPoint0->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	ItemSpawnPoint0->bHiddenInGame = true;

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
	RoomMesh->SetRelativeLocation(FVector(923.0f, -286.94f, 0.0f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	Bonfire = CreateDefaultSubobject<UChildActorComponent>(TEXT("Bonfire"));
	Bonfire->SetupAttachment(Root);
	Bonfire->SetRelativeLocation(FVector(908.0f, -841.0f, 14.0f));
	Bonfire->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	if (BonfireRef.Succeeded())
	{
		Bonfire->SetChildActorClass(BonfireRef.Class);
	}
}
