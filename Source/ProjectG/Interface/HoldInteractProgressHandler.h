// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HoldInteractProgressHandler.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHoldInteractProgressHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTG_API IHoldInteractProgressHandler
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void UpdateHoldProgress(float Progress) = 0;
	virtual void StopHoldProress() = 0;
};
