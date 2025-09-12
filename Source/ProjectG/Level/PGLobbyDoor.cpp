// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/PGLobbyDoor.h"

// Sets default values
APGLobbyDoor::APGLobbyDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/Imports/SICKA_mansion/StaticMeshes/SM_DoorCarved.SM_DoorCarved'"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh0"));
	Mesh0->SetupAttachment(Root);
	Mesh0->SetRelativeLocation(FVector(-163.0f, 22.1f, 0.0f));
	Mesh0->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.96f));

	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh1"));
	Mesh1->SetupAttachment(Root);
	Mesh1->SetRelativeLocation(FVector(163.0f, 0.0f, 0.0f));
	Mesh1->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Mesh1->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.96f));

	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
		Mesh1->SetStaticMesh(MeshRef.Object);
	}
}

TSubclassOf<UGameplayAbility> APGLobbyDoor::GetAbilityToInteract() const
{
	return InteractAbility;
}

/*
* 
*/
void APGLobbyDoor::HighlightOn() const
{
	Mesh0->SetRenderCustomDepth(true);
	Mesh1->SetRenderCustomDepth(true);
}

/*
* 
*/
void APGLobbyDoor::HighlightOff() const
{
	Mesh0->SetRenderCustomDepth(false);
	Mesh1->SetRenderCustomDepth(false);
}

/*
* Lobby Door의 InteractionInfo 반환
* 항상 3초 Hold Input
*/
FInteractionInfo APGLobbyDoor::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Hold, 3.0f);
}
