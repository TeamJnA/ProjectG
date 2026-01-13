// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "PGMirrorRoom.generated.h"

class UBoxComponent;
class UNavModifierComponent;
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

	void StartGimmick();
	void SpawnMirrorGhost(APGPlayerCharacter* Player, const FTransform& SpawnTransforms);

	void UpdateGateMovement();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetGateState(bool bLock);

	UPROPERTY(EditDefaultsOnly, Category = "Gate")
	FVector GateOpenRelativeLocation = FVector(15.0f, 0.0f, 320.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Gate")
	FVector GateClosedRelativeLocation = FVector(15.0f, 0.0f, 6.0f);

	FVector GateStartLoc;
	FVector GateTargetLoc;

	UPROPERTY()
	TArray<TObjectPtr<APGMirrorGhostCharacter>> SpawnedGhosts;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<APGMirrorGhostCharacter> MirrorGhostClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> EntryTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<USceneComponent> MirrorGhostSpawnPointFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UArrowComponent> MirrorGhostSpawnPoint0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UArrowComponent> MirrorGhostSpawnPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UArrowComponent> MirrorGhostSpawnPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UArrowComponent> MirrorGhostSpawnPoint3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UChildActorComponent> LeverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nav")
	TObjectPtr<UNavModifierComponent> NavModifier;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorCloseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorOpenSound;

	FTimerHandle LockTriggerTimerHandle;
	FTimerHandle GateMoveTimerHandle;

	float CurrentGateTime = 0.0f;
	float GateMoveDuration = 1.0f;

	bool bIsLocked = false;
	bool bIsSolved = false;
};
