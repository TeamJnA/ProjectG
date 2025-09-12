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
	void CheckOverlap(const TObjectPtr<USceneComponent>& InSelectedExitPoint, TObjectPtr<APGMasterRoom> RoomToCheck);
	bool IsLatestRoomOverlapping(const APGMasterRoom* RoomToCheck) const;
	void SetupLevelEnvironment();

	void CloseHoles();
	void SpawnDoors();
	void SpawnItems();
	void SpawnEnemy();
	void SpawnSingleItem_Async(int32 ItemAmount);

	void StartLevelGenerateTimer() const;
	void CheckLevelGenerateTimeOut() const;

	const APGMasterRoom* FindFarthestRoom() const;
	const APGMasterRoom* FindMiddleDistanceRoom() const;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TMap<TObjectPtr<APGMasterRoom>, TArray<TObjectPtr<APGMasterRoom>>> RoomGraph;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ExitPointsList;
	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> DoorPointsList;
	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ItemSpawnPointsList;	
	UPROPERTY()
	TArray<TSubclassOf<class APGMasterRoom>> RoomsList;

	UPROPERTY()
	TSubclassOf<APGBlindCharacter> BlindCharacter;

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
};
