// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Level/Searchable/PGSearchableSpawnPoint.h"
#include "Level/Misc/Component/PGGimmickSpawnPoint.h"
#include "PGLevelGenerator.generated.h"

class APGMasterRoom;
class APGBlindCharacter;
class APGChargerCharacter;
class APGDoor1;
class APGFuseBox;
class APGWaiterStand;
class APGHideProp;
class APGSearchableBase;
class APGSearchableSlotBase;
class UPGBloodstainSpawnPoint;

UENUM()
enum class EGimmickSpawnMode : uint8
{
	// 고정 개수(SpawnCount)만큼, 서로 최대한 멀게 배치
	SpreadByCount	UMETA(DisplayName = "Spread By Count"),
	// 후보 대비 비율(SpawnRatio)만큼, 단순 랜덤 배치
	RandomByRatio	UMETA(DisplayName = "Random By Ratio")
};

USTRUCT()
struct FGimmickSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> GimmickClass;

	// 선택되지 않은 포인트에 스폰할 클래스 (없으면 스폰 x)
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> FallbackClass;

	UPROPERTY(EditDefaultsOnly)
	EGimmickSpawnMode SpawnMode = EGimmickSpawnMode::SpreadByCount;

	// 0보다 크면 SpawnCount 대신 비율로 개수 결정
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "SpawnMode == EGimmickSpawnMode::RandomByRatio", ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnRatio = 0.5f;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "SpawnMode == EGimmickSpawnMode::SpreadByCount", ClampMin = "0"))
	int32 SpawnCount = 2;

	// 이 depth 미만의 방은 후보에서 제외 (0이면 제한 x)
	UPROPERTY(EditDefaultsOnly)
	int32 MinRoomDepth = 0;
};

UCLASS()
class PROJECTG_API APGLevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGLevelGenerator();

protected:
	// room
	void SetSeed();
	void SpawnStartRoom();
	void SpawnLoopCorridor();
	void SpawnSingleLoopCorridor(TSubclassOf<APGMasterRoom> LoopClass);

	void SpawnNextRoom();
	TSubclassOf<APGMasterRoom> GetNextRoomClass() const;
	void BuildExitPointWeights(TArray<float>& OutWeights) const;
	int32 PickWeightedIndex(const TArray<float>& Weights, float TotalWeight) const;
	bool WouldRoomOverlap(TSubclassOf<APGMasterRoom> RoomClass, const FTransform& InSpawnTransform) const;
	void RegisterSpawnedRoom(TObjectPtr<USceneComponent> InSelectedExitPoint, TObjectPtr<APGMasterRoom> NewRoom);
	void ScheduleNextRoomSpawn();
	int32 GetRoomDepthFromStart(const APGMasterRoom* Room) const;

	// environment
	void SetupLevelEnvironment();

	void CloseHoles();
	void SpawnDoors();
	void SpawnSearchables();

	void SpawnItems();
	void SpawnExitItems();
	void SpawnItemAtSlot(const FName& ItemKey, APGSearchableSlotBase* Slot);
	APGSearchableSlotBase* AcquireExitItemSlot(
		int32 MinDepth,
		TSet<TObjectPtr<APGMasterRoom>>& UsedRooms,
		TSet<TObjectPtr<APGMasterRoom>>& UsedBranches,
		TSet<TObjectPtr<APGSearchableBase>>& UsedSearchables);
	APGMasterRoom* GetBranchRoot(APGMasterRoom* Room) const;

	void SpawnGimmicks();
	void CollectGimmickCandidates(
		EGimmickType GimmickType,
		const FGimmickSpawnConfig& Config,
		TArray<TObjectPtr<UPGGimmickSpawnPoint>>& OutCandidates) const;
	int32 ResolveGimmickSpawnCount(const FGimmickSpawnConfig& Config, int32 CandidateCount) const;
	void SelectPointsRandom(
		TArray<TObjectPtr<UPGGimmickSpawnPoint>>& Candidates,
		int32 SelectCount,
		TArray<TObjectPtr<UPGGimmickSpawnPoint>>& OutSelected) const;
	void SelectPointsMaxSpread(
		TArray<TObjectPtr<UPGGimmickSpawnPoint>>& Candidates,
		int32 SelectCount,
		TArray<TObjectPtr<UPGGimmickSpawnPoint>>& OutSelected) const;
	APGMasterRoom* GetGimmickPointOwnerRoom(const TObjectPtr<UPGGimmickSpawnPoint>& Point) const;

	void SpawnFuseBoxes();
	void SpawnWaiterStands();
	void SpawnHideProps();
	void SpawnBloodStains();
	bool SpawnEnemy();
	void SpawnSingleItem_Async(int32 ItemAmount, int32 SeqIndex);

	void StartLevelGenerateTimer();
	void CheckLevelGenerateTimeOut();
	void ReGenerateLevel();

	void EnsureRoomDepthMap();
	void BuildRoomDepthMap();
	void BuildHopDistanceFrom(APGMasterRoom* Origin, TMap<TObjectPtr<APGMasterRoom>, int32>& OutDist) const;
	const APGMasterRoom* FindFarthestRoom() const;
	const APGMasterRoom* FindMiddleDistanceRoom(const FVector& AvoidLocation = FVector::ZeroVector) const;
	void ComputeExplorationWaypoints();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void AddPropsSpawnPoint(TObjectPtr<APGMasterRoom> RoomToCheck);
	void AddGimmickSpawnPoints(TObjectPtr<APGMasterRoom> Room);

	TObjectPtr<USceneComponent> GetRandomPointFromSpecificListAndRemove(TArray<TObjectPtr<USceneComponent>>& TargetList, TArray<TObjectPtr<USceneComponent>>& TargetRemoveList);

	UPROPERTY(EditDefaultsOnly, Category = "Level Generation", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSubclassOf<APGMasterRoom>> RoomClassMap;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick", meta = (AllowPrivateAccess = "true"))
	TMap<EGimmickType, FGimmickSpawnConfig> GimmickConfigMap;

	TMap<TObjectPtr<APGMasterRoom>, TArray<TObjectPtr<APGMasterRoom>>> RoomGraph;

	// Room -> StartRoom 으로부터의 깊이
	UPROPERTY()
	TMap<TObjectPtr<APGMasterRoom>, int32> RoomDepths;

	// Room -> BFS 트리상의 부모 (StartRoom 은 미포함)
	UPROPERTY()
	TMap<TObjectPtr<APGMasterRoom>, TObjectPtr<APGMasterRoom>> RoomParents;

	// Enum으로만 해당 Searchable을 스폰하기 위한 맵
	UPROPERTY(EditDefaultsOnly, Category = "Searchable", meta = (AllowPrivateAccess = "true"))
	TMap<ESearchableType, TSubclassOf<APGSearchableBase>> SearchableClassMap;

	// Searchable -> 소속 Room
	UPROPERTY()
	TMap<TObjectPtr<APGSearchableBase>, TObjectPtr<APGMasterRoom>> SearchableOwnerRooms;

	UPROPERTY(EditDefaultsOnly, Category = "Level Generation", meta = (AllowPrivateAccess = "true"))
	TArray<FName> ExitItemKeys = { FName("ChainKey"), FName("HandWheel"), FName("RustOil") };

	UPROPERTY(EditDefaultsOnly, Category = "Level Generation", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<APGMasterRoom>> LoopCorridorClassArray;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> ExitPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> DoorPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> SearchableSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<UPGGimmickSpawnPoint>> GimmickSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> FuseBoxSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> WaiterStandSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> GlassBottleSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<USceneComponent>> HidePropSpawnPointsList;

	UPROPERTY()
	TArray<TObjectPtr<UPGBloodstainSpawnPoint>> BloodstainSpawnPointsList;

	UPROPERTY(EditDefaultsOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APGFuseBox> FuseBoxClass;

	UPROPERTY(EditDefaultsOnly, Category = "Props", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APGWaiterStand> WaiterStandClass;

	UPROPERTY(EditDefaultsOnly, Category = "Props", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<APGHideProp>> HidePropClasses;

	UPROPERTY()
	TArray<TObjectPtr<APGSearchableBase>> SpawnedSearchables;

	UPROPERTY()
	TSubclassOf<APGBlindCharacter> BlindCharacter;

	UPROPERTY()
	TSubclassOf<APGChargerCharacter> ChargerCharacter;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<APGDoor1> PGDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<APGMasterRoom> StartRoom;

	UPROPERTY()
	FRandomStream Seed;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick", meta = (AllowPrivateAccess = "true"))
	float ApproxRoomSpacing = 2000.0f;

	float GenerationStartTime;
	float MaxGenerateTime;
	int32 SeedValue = -1;
	int32 RoomAmount;

	bool bRoomDepthsDirty = true;
	bool bIsGenerationStopped = false;

	//debug
	double AccumSpawnNextRoom = 0.0;
	double AccumCheckOverlap = 0.0;
	int32 RoomSpawnAttempts = 0;
	int32 RoomOverlapFails = 0;
};
