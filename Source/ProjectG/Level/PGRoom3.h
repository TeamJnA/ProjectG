// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGRoom3.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGRoom3 : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGRoom3();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	TObjectPtr<UArrowComponent> SpawnPoint3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;
};
