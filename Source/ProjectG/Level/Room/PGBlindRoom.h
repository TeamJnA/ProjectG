// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "Type/PGPhotoTypes.h"
#include "PGBlindRoom.generated.h"

class APGSwingProp;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGBlindRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGBlindRoom();
	virtual void SpawnPhotoSpots() override;
	virtual void SpawnSwingProps(const FRandomStream& InStream) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TArray<FPhotoSpotConfig> PhotoSpotConfigs;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> SwingPropPointsFolder;

	UPROPERTY(EditDefaultsOnly, Category = "SwingProp")
	TSubclassOf<APGSwingProp> SwingPropClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> RoomMesh;

	UPROPERTY(EditDefaultsOnly, Category = "SwingProp")
	int32 SwingPropSpawnCount = 6;
};
