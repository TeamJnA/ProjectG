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
	// void AddFloorSpawnPointsToList();
	void CheckOverlap();
	void CloseHoles();
	void SpawnDoors();

	void StartDungeonTimer();
	void CheckForDungeonComplete();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SelectedExitPoint", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SelectedExitPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SelectedDoorPoint", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SelectedDoorPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DoorPointsList", meta = (AllowPrivateAccess = "true"))
	TArray<USceneComponent*> DoorPointsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExitsList", meta = (AllowPrivateAccess = "true"))
	TArray<USceneComponent*> ExitsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OverlappedList", meta = (AllowPrivateAccess = "true"))
	TArray<UPrimitiveComponent*> OverlappedList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class APGMasterRoom>> RoomsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class APGMasterRoom>> SpecialRoomsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RoomsList", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class APGMasterRoom>> BaseRoomsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SeedValue", meta = (AllowPrivateAccess = "true"))
	int SeedValue = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LatestRoom", meta = (AllowPrivateAccess = "true"))
	APGMasterRoom* LatestRoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaxRoomAmount", meta = (AllowPrivateAccess = "true"))
	int RoomAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MaxDoorAmount", meta = (AllowPrivateAccess = "true"))
	int DoorAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IsGenerationDone", meta = (AllowPrivateAccess = "true"))
	bool bIsGenerationDone;

private:
	FRandomStream Seed;
	FTimerHandle TimerHandler;
	float MaxGenerateTime;

};
