// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGRoom2.generated.h"

class UArrowComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGRoom2 : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGRoom2();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;

};
