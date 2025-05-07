// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectG/Enemy/Base/PGEnemyCharacterBase.h"

UBehaviorTree* APGEnemyCharacterBase::GetBehaviorTree() const
{
	return Tree;
}

APGPatrolPath* APGEnemyCharacterBase::GetPatrolPath() const
{
	return PatrolPath;
}
