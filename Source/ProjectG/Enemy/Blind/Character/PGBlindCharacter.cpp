// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Character/PGBlindCharacter.h"

float APGBlindCharacter::GetExplorationRadius() const
{
    return ExplorationRadius;
}

float APGBlindCharacter::GetExplorationWaitTime() const
{
    return ExplorationWaitTime;
}

float APGBlindCharacter::GetExplorationMoveSpeed() const
{
    return ExplorationMoveSpeed;
}
