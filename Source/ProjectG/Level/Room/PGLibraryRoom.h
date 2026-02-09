// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGLibraryRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGLibraryRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGLibraryRoom();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;
};
