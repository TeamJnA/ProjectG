// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGBarrelRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGBarrelRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGBarrelRoom();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bonfire")
	TObjectPtr<UChildActorComponent> Bonfire;
};
