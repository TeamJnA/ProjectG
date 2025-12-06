// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Interface/HoldInteractProgressHandler.h"
#include "PGExitIronDoor.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class E_LockPhase : uint8
{
	E_ChainLock,  // 0. 체인 잠금을 해제해야 하는 상태
	E_WheelAttach, // 1. 휠을 장착해야 하는 상태
	E_OilApplied, // 2. 윤활유를 발라야 하는 상태
	E_Unlocked    // 3. 잠금 해제 완료 상태
};

UCLASS()
class PROJECTG_API APGExitIronDoor : public APGExitPointBase, public IHoldInteractProgressHandler
{
	GENERATED_BODY()

public:
	APGExitIronDoor();

	// IInteractableActorInterface~
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	// IHoldInteractProgressHandler~
	virtual void UpdateHoldProgress(float Progress) override;
	virtual void StopHoldProress() override;
	// ~IHoldInteractProgressHandler

	virtual void Unlock() override;

private:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void InitializeChainComponents();

	void SetChainsUnlock();

	void SetWheelMaterialOiled();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PillarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> IronDoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> IronChainMesh;
	
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ChainMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HandWheelHole;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HandWheelLubricantPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> HandWheelOiledMaterial;

	E_LockPhase CurrentLockPhase;

	/*
	Door open properties
	*/
	FVector DoorBaseLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IronDoor", meta = (AllowPrivateAccess = "true"))
	float MaxDoorHeight = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IronDoor", meta = (AllowPrivateAccess = "true"))
	float CurrentDoorHeight = 0.0f;

	bool bDoorAutoClose;
};
