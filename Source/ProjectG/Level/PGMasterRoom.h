// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGMasterRoom.generated.h"

class UArrowComponent;
class UBoxComponent;

UCLASS()
class PROJECTG_API APGMasterRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGMasterRoom();

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
	TObjectPtr<USceneComponent> ExitsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> FloorSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EnemySpawnPoint")
	TObjectPtr<UArrowComponent> EnemySpawnPoint;

public:
	virtual USceneComponent* GetExitsFolder();
	virtual USceneComponent* GetOverlapBoxFolder();
	virtual USceneComponent* GetFloorSpawnPointsFolder();
	virtual FVector GetEnemySpawnLocation();

};
