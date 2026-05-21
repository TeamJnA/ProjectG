// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Searchable/PGSearchableSlotBase.h"

// Sets default values
APGSearchableSlotBase::APGSearchableSlotBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SlotMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh1"));
	SetRootComponent(SlotMesh1);

	SlotMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh2"));
	SlotMesh2->SetupAttachment(SlotMesh1);

	ItemSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSpawnPoint"));
	ItemSpawnPoint->SetupAttachment(SlotMesh1);
}

// Called when the game starts or when spawned
void APGSearchableSlotBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APGSearchableSlotBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

