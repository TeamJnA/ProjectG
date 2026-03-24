// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGHideProp.generated.h"

UCLASS()
class PROJECTG_API APGHideProp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGHideProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
