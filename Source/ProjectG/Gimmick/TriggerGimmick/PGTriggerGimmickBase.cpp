// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"

// Sets default values
APGTriggerGimmickBase::APGTriggerGimmickBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APGTriggerGimmickBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APGTriggerGimmickBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

