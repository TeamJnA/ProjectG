// Fill out your copyright notice in the Description page of Project Settings.


#include "PGWall.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
APGWall::APGWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/BasicAssets/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialRef(TEXT("/Script/Engine.Material'/Game/BasicAssets/M_Basic_Floor.M_Basic_Floor'"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Root->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Root->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Root->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh0"));
	Mesh0->SetupAttachment(Root);
	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
	}
	if (MaterialRef.Object)
	{
		Mesh0->SetMaterial(0, MaterialRef.Object);
	}
	Mesh0->SetRelativeLocation(FVector(-16.0f, 0.0f, 25.0f));
	Mesh0->SetRelativeScale3D(FVector(0.3f, 3.0f, 0.7f));
}
