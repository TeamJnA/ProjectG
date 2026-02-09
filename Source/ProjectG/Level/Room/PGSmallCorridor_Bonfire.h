// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGSmallCorridor_Bonfire.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGSmallCorridor_Bonfire : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGSmallCorridor_Bonfire();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bonfire")
	TObjectPtr<UChildActorComponent> Bonfire;
};
