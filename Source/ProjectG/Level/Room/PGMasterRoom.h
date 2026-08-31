// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/ArrowComponent.h"
#include "Type/PGStartRoomTypes.h"

#include "PGMasterRoom.generated.h"

class UBoxComponent;

USTRUCT()
struct FPGRoomOverlapBox
{
	GENERATED_BODY()

	FVector Location = FVector::ZeroVector;
	FQuat Rotation = FQuat::Identity;
	FVector HalfExtent = FVector::ZeroVector;
};

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
	FORCEINLINE const USceneComponent* GetGimmickSpawnPointsFolder() const { return GimmickSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetFuseBoxSpawnPointsFolder() const { return FuseBoxSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetPropsSpawnPointsFolder() const { return PropsSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetGlassBottleSpawnPointsFolder() const { return GlassBottleSpawnPointsFolder; }
	FORCEINLINE const USceneComponent* GetBloodstainSpawnPointsFolder() const { return BloodstainSpawnPointsFolder; }

	FORCEINLINE FVector GetEnemySpawnLocation() const { return EnemySpawnPoint->GetComponentLocation(); }
	FORCEINLINE TSubclassOf<AActor> GetWallClass() const { return WallClass; }

	FORCEINLINE EStartRoomExit GetLoopStartExit() const { return LoopStartExit; }
	FORCEINLINE EStartRoomExit GetLoopEndExit() const { return LoopEndExit; }
	FORCEINLINE bool IsSecondFloorLoop() const { return bIsSecondFloorLoop; }

	virtual void SpawnPhotoSpots() {}
	virtual void SpawnSwingProps(const FRandomStream& InStream) {}

	/*
	* OutBoxes를 Room의 OverlapBox들로 채우기
	* 배치 가능 여부를 사전 판정할 때 사용
	* *블루프린트 에디터에서 추가한 박스는 CDO에 존재하지 않아 잡히지 않음
	*/
	static void GetOverlapBoxesForClass(
		TSubclassOf<APGMasterRoom> RoomClass,
		const FTransform& InRoomTransform,
		TArray<FPGRoomOverlapBox>& OutBoxes);

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
	TObjectPtr<USceneComponent> GimmickSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> FuseBoxSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> PropsSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> GlassBottleSpawnPointsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	TObjectPtr<USceneComponent> BloodstainSpawnPointsFolder;

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
