// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/ArrowComponent.h"
#include "Type/PGStartRoomTypes.h"

#include "PGMasterRoom.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTG_API APGMasterRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGMasterRoom();

	FORCEINLINE const USceneComponent* GetExitPointsFolder() const { return ExitPointsFolder; }
	FORCEINLINE const USceneComponent* GetOverlapBoxFolder() const { return OverlapBoxFolder; }
	FORCEINLINE const USceneComponent* GetSearchableSpawnPointsFolder() const { return SearchableSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetMannequinSpawnPointsFolder() const { return MannequinSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetArmorStandSpawnPointsFolder() const { return ArmorStandSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetFuseBoxSpawnPointsFolder() const { return FuseBoxSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetPropsSpawnPointsFolder() const { return PropsSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetGlassBottleSpawnPointsFolder() const { return GlassBottleSpawnPointsFolder; }

	FORCEINLINE FVector GetEnemySpawnLocation() const { return EnemySpawnPoint->GetComponentLocation(); }
	FORCEINLINE TSubclassOf<AActor> GetWallClass() const { return WallClass; }

	FORCEINLINE EStartRoomExit GetLoopStartExit() const { return LoopStartExit; }
	FORCEINLINE EStartRoomExit GetLoopEndExit() const { return LoopEndExit; }
	FORCEINLINE bool IsSecondFloorLoop() const { return bIsSecondFloorLoop; }

	virtual void SpawnPhotoSpots() {}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> GeometryFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomDirection")
	TObjectPtr<UArrowComponent> RoomDir;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> OverlapBoxFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OverlapBox")
	TObjectPtr<UBoxComponent> OverlapBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> ExitPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> SearchableSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> MannequinSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> ArmorStandSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> FuseBoxSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> PropsSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> GlassBottleSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EnemySpawnPoint")
	TObjectPtr<UArrowComponent> EnemySpawnPoint;

	UPROPERTY(EditDefaultsOnly, Category = "LevelGeneration")
	TSubclassOf<AActor> WallClass;

	// for loop corridor only
	UPROPERTY(EditDefaultsOnly, Category = "Loop Corridor")
	EStartRoomExit LoopStartExit = EStartRoomExit::None;

	UPROPERTY(EditDefaultsOnly, Category = "Loop Corridor")
	EStartRoomExit LoopEndExit = EStartRoomExit::None;

	UPROPERTY(EditDefaultsOnly, Category = "Loop Corridor")
	bool bIsSecondFloorLoop = false;
};
