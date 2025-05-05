// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/PGPatrolPath.h"

// Sets default values
APGPatrolPath::APGPatrolPath()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

FVector APGPatrolPath::GetPatrolPoint(int const index) const
{
	return PatrolPoints[index];
}

int APGPatrolPath::Num() const
{
	return PatrolPoints.Num();
}

