// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LightEffectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULightEffectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTG_API ILightEffectInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void FadeOut() = 0;
	virtual void FadeIn() = 0;
};
