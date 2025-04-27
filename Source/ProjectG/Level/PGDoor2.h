// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGDoor2.generated.h"

UCLASS()
class PROJECTG_API APGDoor2 : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGDoor2();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WallMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WallMesh")
	TObjectPtr<UStaticMeshComponent> Mesh1;

};
