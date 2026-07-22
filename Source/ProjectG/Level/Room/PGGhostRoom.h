// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "Type/PGPhotoTypes.h"
#include "PGGhostRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGhostRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGGhostRoom();
	virtual void SpawnPhotoSpots() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FPhotoSpotConfig PhotoSpotConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FuseBoxSpawnPoint")
	TObjectPtr<UArrowComponent> FuseBoxSpawnPoint0;
};
