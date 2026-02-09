// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGWall_Brown.generated.h"

UCLASS()
class PROJECTG_API APGWall_Brown : public AActor
{
	GENERATED_BODY()
	
public:
	APGWall_Brown();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WallMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

};
