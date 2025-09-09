// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGLevelGenerator.generated.h"

class APGMasterRoom;
class APGBlindCharacter;

UCLASS()
class PROJECTG_API APGLevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGLevelGenerator();

	const TMap<TObjectPtr<APGMasterRoom>, TArray<TObjectPtr<APGMasterRoom>>>& GetRoomGraph() const { return RoomGraph; }

protected:
	void SetSeed();
	void SpawnStartRoom();
	void SpawnNextRoom();
	void CheckOverlap(TObjectPtr<USceneComponent> InSelectedExitPoint);
	void SetupLevelEnvironment();

	void AddOverlappingRoomsToList();
	void AddFloorSpawnPointsToList();

	void CloseHoles();
	void SpawnDoors();
	void SpawnItems();
	void SpawnSingleItem_Async();

	void StartDungeonTimer();
	void CheckForDungeonComplete();

	APGMasterRoom* FindFarthestRoom() const;
	APGMasterRoom* FindMiddleDistanceRoom() const;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TMap<TObjectPtr<APGMasterRoom>, TArray<TObjectPtr<APGMasterRoom>>> RoomGraph;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ExitsList;
	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> DoorPointsList;
	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> FloorSpawnPointsList;
	UPROPERTY()
	TArray<TObjectPtr<UPrimitiveComponent>> OverlappedList;
	UPROPERTY()
	TArray<TSubclassOf<class APGMasterRoom>> RoomsList;

	UPROPERTY()
	TSubclassOf<APGBlindCharacter> BlindCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<APGMasterRoom> StartRoom;
	UPROPERTY()
	TObjectPtr<APGMasterRoom> LatestRoom;

	UPROPERTY()
	FRandomStream Seed;

	FTimerHandle TimerHandler;

	float GenerationStartTime;
	float MaxGenerateTime;
	int32 SeedValue = -1;
	int32 RoomAmount;
	int32 DoorAmount;
	int32 LockedDoorAmount;
	int32 ItemAmount;
};
