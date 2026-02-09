// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGChargerRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGChargerRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGChargerRoom();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OverlapBox")
	TObjectPtr<UBoxComponent> OverlapBox1;
};
