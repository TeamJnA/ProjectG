// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGWall.generated.h"

UCLASS()
class PROJECTG_API APGWall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGWall();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WallMesh")
	class UStaticMeshComponent* Mesh0;

};
