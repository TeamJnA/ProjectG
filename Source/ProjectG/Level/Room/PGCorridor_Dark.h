// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGCorridor_Dark.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGCorridor_Dark : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGCorridor_Dark();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;
};
