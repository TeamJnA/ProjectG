// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGEnemyCharacter.h"

UBehaviorTree* APGEnemyCharacter::GetBehaviorTree() const
{
	return Tree;
}

APGPatrolPath* APGEnemyCharacter::GetPatrolPath() const
{
	return PatrolPath;
}
