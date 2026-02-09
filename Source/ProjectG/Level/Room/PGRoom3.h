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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ItemSpawnPoint")
	TObjectPtr<UArrowComponent> ItemSpawnPoint4;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MannequinSpawnPoint")
	TObjectPtr<UArrowComponent> MannequinSpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick3;
};
