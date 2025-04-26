// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGLevelGenerator.generated.h"

class APGMasterRoom;

UCLASS()
class PROJECTG_API APGLevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGLevelGenerator();

protected:
	void SetSeed();
	void SpawnStartRoom();
	void SpawnNextRoom();

	void AddOverlappingRoomsToList();
	void AddFloorSpawnPointsToList();
	void CheckOverlap();
	void CloseHoles();
	void SpawnDoors();
	void SpawnItems();

	UFUNCTION()
	void StartDungeonTimer();

	UFUNCTION()
	void CheckForDungeonComplete();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SelectedExitPoint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SelectedExitPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SelectedDoorPoint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SelectedDoorPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SelectedFloorSpawnPoint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SelectedFloorSpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DoorPointsList", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USceneComponent>> DoorPointsList;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FloorSpawnPointsList", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USceneComponent>> FloorSpawnPointsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExitsList", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USceneComponent>> ExitsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OverlappedList", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UPrimitiveComponent>> OverlappedList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class APGMasterRoom>> RoomsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpecialRoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class APGMasterRoom>> SpecialRoomsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseRoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class APGMasterRoom>> BaseRoomsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class AActor>> ItemsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SeedValue", meta = (AllowPrivateAccess = "true"))
	int SeedValue = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LatestRoom", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APGMasterRoom> LatestRoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaxRoomAmount", meta = (AllowPrivateAccess = "true"))
	int RoomAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaxDoorAmount", meta = (AllowPrivateAccess = "true"))
	int DoorAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaxItemAmount", meta = (AllowPrivateAccess = "true"))
	int ItemAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IsGenerationDone", meta = (AllowPrivateAccess = "true"))
	bool bIsGenerationDone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LevelSeed", meta = (AllowPrivateAccess = "true"))
	FRandomStream Seed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimerHandle", meta = (AllowPrivateAccess = "true"))
	FTimerHandle TimerHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GenerationStartTime", meta = (AllowPrivateAccess = "true"))
	float GenerationStartTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaxGenerateTime", meta = (AllowPrivateAccess = "true"))
	float MaxGenerateTime;

};
