// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGCorridor_Simple.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGCorridor_Simple : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGCorridor_Simple();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir4;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;
};
