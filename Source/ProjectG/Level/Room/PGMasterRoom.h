// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/ArrowComponent.h"

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
	FORCEINLINE const USceneComponent* GetItemSpawnPointsFolder() const { return ItemSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetMannequinSpawnPointsFolder() const { return MannequinSpawnPointsFolder; }
	FORCEINLINE FVector GetEnemySpawnLocation() const { return EnemySpawnPoint->GetComponentLocation(); }
	FORCEINLINE TSubclassOf<AActor> GetWallClass() const { return WallClass; }

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
	TObjectPtr<USceneComponent> ItemSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> MannequinSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EnemySpawnPoint")
	TObjectPtr<UArrowComponent> EnemySpawnPoint;

	UPROPERTY(EditDefaultsOnly, Category = "LevelGeneration")
	TSubclassOf<AActor> WallClass;
};
