// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Interface/HoldInteractProgressHandler.h"
#include "PGExitIronDoor.generated.h"

/**
 * 
 */
class USoundCue;

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IInteractableActorInterface~
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	virtual void InteractionFailed() override;
	// ~IInteractableActorInterface

	// IHoldInteractProgressHandler~
	virtual void UpdateHoldProgress(float Progress) override;
	virtual void StopHoldProress() override;
	// ~IHoldInteractProgressHandler

	virtual bool Unlock() override;

private:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetDoorBaseLocation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnlockChains();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachWheel();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetWheelMaterialOiled();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PillarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> IronDoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> IronChainMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> IronChain1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> IronChain2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HandWheelHole;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interact|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HandWheelLubricantPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> HandWheelOiledMaterial;

	E_LockPhase CurrentLockPhase;


	/*
	* Dynamic materials to make shake effect
	*/
	UPROPERTY(ReplicatedUsing = OnRep_InitMIDs)
	bool bInitMIDs = false;

	UFUNCTION()
	void InitMIDs();

	UFUNCTION()
	void OnRep_InitMIDs();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ActivateShakeEffect(const TArray<UMaterialInstanceDynamic*>& TargetMIDs);

	UFUNCTION()
	void DisableShakeEffect(const TArray<UMaterialInstanceDynamic*>& TargetMIDs);

	UFUNCTION()
	void ToggleShakeEffect(const TArray<UMaterialInstanceDynamic*>& TargetMIDs, bool bToggle);

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDChainLock;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDIronChain1;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDIronChain2;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDWheel;

	FTimerHandle ShakeEffectTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Material Effect", meta = (AllowPrivateAccess = "true"))
	FName TargetParameterName = TEXT("WPOPower");

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> ChainsToShake;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> WheelToShake;

	/*
	Door open properties
	*/
	FVector DoorBaseLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IronDoor", meta = (AllowPrivateAccess = "true"))
	float MaxDoorHeight = 300.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentDoorHeight, VisibleAnywhere, BlueprintReadOnly, Category = "IronDoor", meta = (AllowPrivateAccess = "true"))
	float CurrentDoorHeight = 0.0f;

	UFUNCTION()
	void OnRep_CurrentDoorHeight();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWheelQuat, VisibleAnywhere, BlueprintReadOnly, Category = "IronDoor", meta = (AllowPrivateAccess = "true"))
	FQuat CurrentWheelQuat;

	UFUNCTION()
	void OnRep_CurrentWheelQuat();

	bool bDoorAutoClose;

	UFUNCTION()
	void DoorForceClose();

	bool bDoorForceOpen;

	float DoorAutoCloseSpeed;

	FTimerHandle DoorForceOpenTimerHandle;

	FTimerHandle ChainDropTimerHandle;

	// Sound Managing
	TBitArray<FDefaultBitArrayAllocator> SoundPlayChecker;

	void CleanSoundChecker();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName IronDoorMeshBaseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName IronDoorMeshRustySound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName WheelRotateRustySound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName CannotRotateWheelSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName CannotUnlockChainSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorClosedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName UnlockChainSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName ChainDropSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName WheelAttachedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName OilAppliedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorCloseStartSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> CloseCountSoundCue;

	UPROPERTY()
	TObjectPtr<UAudioComponent> CloseCountSoundAudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartCloseCountSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopCloseCountSound();

	UFUNCTION()
	void PlayChainDropSound();
};
