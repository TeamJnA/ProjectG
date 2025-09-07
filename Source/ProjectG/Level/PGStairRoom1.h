// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGStairRoom1.generated.h"

class UArrowComponent;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGStairRoom1 : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGStairRoom1();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OverlapBox")
	TObjectPtr<UBoxComponent> OverlapBox1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;
};
