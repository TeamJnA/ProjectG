// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/PGHideProp.h"

// Sets default values
APGHideProp::APGHideProp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

// Called when the game starts or when spawned
void APGHideProp::BeginPlay()
{
	Super::BeginPlay();
	
}
