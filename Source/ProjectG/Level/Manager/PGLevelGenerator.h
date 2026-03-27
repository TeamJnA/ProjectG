// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGLevelGenerator.generated.h"

class APGMasterRoom;
class APGBlindCharacter;
class APGChargerCharacter;
class APGTriggerGimmickMannequin;
class APGDoor1;
class APGFuseBox;
class APGWaiterStand;
class APGHideProp;

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
	void CheckOverlap(TObjectPtr<USceneComponent> InSelectedExitPoint, TObjectPtr<APGMasterRoom> RoomToCheck);
	bool IsLatestRoomOverlapping(const APGMasterRoom* RoomToCheck) const;
	void SetupLevelEnvironment();

	void CloseHoles();
	void SpawnDoors();
	void SpawnItems();
	void SpawnMannequins();
	void SpawnFuseBoxes();
	void SpawnWaiterStands();
	void SpawnHideProps();
	bool SpawnEnemy();
	void SpawnSingleItem_Async(int32 ItemAmount);

	void StartLevelGenerateTimer();
	void CheckLevelGenerateTimeOut();
	void ReGenerateLevel();

	const APGMasterRoom* FindFarthestRoom() const;
	const APGMasterRoom* FindMiddleDistanceRoom(const FVector& AvoidLocation = FVector::ZeroVector) const;
	void ComputeExplorationWaypoints();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void AddItemSpawnPoint(TObjectPtr<APGMasterRoom> RoomToCheck);

	void AddPropsSpawnPoint(TObjectPtr<APGMasterRoom> RoomToCheck);

	TObjectPtr<USceneComponent> GetRandomPointFromSpecificListAndRemove(TArray<TObjectPtr<USceneComponent>>& TargetList, TArray<TObjectPtr<USceneComponent>>& TargetRemoveList);

	UPROPERTY(EditDefaultsOnly, Category = "Level Generation", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSubclassOf<APGMasterRoom>> RoomClassMap;

	TMap<TObjectPtr<APGMasterRoom>, TArray<TObjectPtr<APGMasterRoom>>> RoomGraph;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ExitPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> DoorPointsList;

	// Item Spawn ~
	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ItemSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ExitKeyPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> HandWheelPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> RustOilPointsList;
	// ~

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> MannequinSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> FuseBoxSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> WaiterStandSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> HidePropSpawnPointsList;

	UPROPERTY(EditDefaultsOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APGFuseBox> FuseBoxClass;

	UPROPERTY(EditDefaultsOnly, Category = "Props", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APGWaiterStand> WaiterStandClass;

	UPROPERTY(EditDefaultsOnly, Category = "Props", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<APGHideProp>> HidePropClasses;

	UPROPERTY()
	TSubclassOf<APGBlindCharacter> BlindCharacter;

	UPROPERTY()
	TSubclassOf<APGChargerCharacter> ChargerCharacter;

	UPROPERTY()
	TSubclassOf<APGTriggerGimmickMannequin> MannequinClass;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<APGDoor1> PGDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<APGMasterRoom> StartRoom;

	UPROPERTY()
	FRandomStream Seed;

	float GenerationStartTime;
	float MaxGenerateTime;
	int32 SeedValue = -1;
	int32 RoomAmount;

	bool bIsGenerationStopped = false;
};
