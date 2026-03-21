// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Ability/GA_Item_Throw.h"
#include "GA_Throw_Glass.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_Throw_Glass : public UGA_Item_Throw
{
	GENERATED_BODY()

public:
	UGA_Throw_Glass();

protected:
	virtual void SpawnProjectileActor() override;
};
