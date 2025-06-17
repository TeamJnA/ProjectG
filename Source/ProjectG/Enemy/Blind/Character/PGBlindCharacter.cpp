// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/Ability/PGBlindAttributeSet.h"

APGBlindCharacter::APGBlindCharacter()
{
    BlindAttributeSet = CreateDefaultSubobject<UPGBlindAttributeSet>("BlindAttributeSet");

}

float APGBlindCharacter::GetExplorationRadius() const
{
    return ExplorationRadius;
}

float APGBlindCharacter::GetExplorationWaitTime() const
{
    return ExplorationWaitTime;
}



float APGBlindCharacter::GetNoiseLevelThreshold() const
{
    return NoiseLevelThreshold;
}

int APGBlindCharacter::GetHuntLevel() const
{
    return HuntLevel;
}

void APGBlindCharacter::SetHuntLevel(int Level)
{
    HuntLevel = Level;
}


