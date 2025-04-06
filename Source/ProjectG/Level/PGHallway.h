// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGHallway.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGHallway : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGHallway();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	class UArrowComponent* ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh2;	
};
