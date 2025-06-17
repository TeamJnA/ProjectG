// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGAIExplorationInterface.generated.h"

/**
 * 
 */

UINTERFACE(Blueprintable)
class PROJECTG_API UPGAIExplorationInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTG_API IPGAIExplorationInterface
{
	GENERATED_BODY()


public:
	virtual float GetExplorationRadius() const = 0;
	virtual float GetExplorationWaitTime() const = 0;
};
