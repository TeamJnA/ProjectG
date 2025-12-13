// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGMirrorRoom.generated.h"

class UBoxComponent;
class APGMirrorGhostCharacter;
class APGInteractableGimmickLever;
class APGPlayerCharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGMirrorRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGMirrorRoom();

	void SolveGimmick();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> EntryTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<APGMirrorGhostCharacter> GhostClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USceneComponent> GhostSpawnPointFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UChildActorComponent> LeverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Gate")
	float GateMoveDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Gate")
	FVector GateOpenRelativeLocation = FVector(80.0f, -110.0f, 300.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Gate")
	FVector GateClosedRelativeLocation = FVector(80.0f, -110.0f, 0.0f);

private:
	UFUNCTION()
	void OnEntryTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnEntryTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void LockRoomAndSpawnGhosts();
	void SpawnGhostForPlayer(APGPlayerCharacter* Player, const FTransform& SpawnTransforms);

	void UpdateGateMovement();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetGateState(bool bLock);

	UPROPERTY()
	TArray<TObjectPtr<APGMirrorGhostCharacter>> SpawnedGhosts;

	FTimerHandle LockTriggerTimerHandle;
	FTimerHandle GateMoveTimerHandle;
	FVector GateStartLoc;
	FVector GateTargetLoc;
	float CurrentGateTime = 0.0f;

	bool bIsLocked = false;
	bool bIsSolved = false;
};
