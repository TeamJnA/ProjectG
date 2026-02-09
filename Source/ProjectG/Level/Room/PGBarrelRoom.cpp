// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGBarrelRoom.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

APGBarrelRoom::APGBarrelRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> RoomMeshRef(TEXT("/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionBarrelRoom.LI_MansionBarrelRoom_C"));
	static ConstructorHelpers::FClassFinder<AActor> BonfireRef(TEXT("/Game/ProjectG/Gimmick/Interact/Bonfire/BP_PGInteractableGimmickBonfire.BP_PGInteractableGimmickBonfire_C"));
	
	WallClass = APGWall::StaticClass();

	RoomDir->SetRelativeLocation(FVector(300.0f, 0.0f, 130.0f));
	EnemySpawnPoint->SetRelativeLocation(FVector(550.0f, 280.0f, 230.0f));
	OverlapBox->SetRelativeLocation(FVector(620.0f, 255.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(18.5f, 21.0f, 10.5f));

	RoomMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("RoomMesh"));
	RoomMesh->SetupAttachment(GeometryFolder);
	if (RoomMeshRef.Succeeded())
	{
		RoomMesh->SetChildActorClass(RoomMeshRef.Class);
	}
	RoomMesh->SetRelativeLocation(FVector(619.5f, 247.4f, 0.0f));
	RoomMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	Bonfire = CreateDefaultSubobject<UChildActorComponent>(TEXT("Bonfire"));
	Bonfire->SetupAttachment(Root);
	Bonfire->SetRelativeLocation(FVector(603.0f, 850.0f, 18.0f));
	Bonfire->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	if (BonfireRef.Succeeded())
	{
		Bonfire->SetChildActorClass(BonfireRef.Class);
	}
}
