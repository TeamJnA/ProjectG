// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGDiningRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGDiningRoom : public APGMasterRoom
{
	GENERATED_BODY()
	
public:
	APGDiningRoom();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;	
};
