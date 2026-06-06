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
class UBoxComponent;

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
	virtual FText GetInteractionText() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	virtual void InteractionFailed() override;
	// ~IInteractableActorInterface

	void SelfHighlightOff();

	// IHoldInteractProgressHandler~
	virtual void UpdateHoldProgress(float Progress, AActor* Investigator) override;
	virtual void StopHoldProress() override;
	// ~IHoldInteractProgressHandler

	virtual bool Unlock(AActor* Investigator) override;
	virtual TSet<FName> GetUnlockedItemIds() const override;

private:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnlockChains();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachWheel();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetWheelMaterialOiled();

	UFUNCTION()
	bool IsLocalPlayerLookingThis() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PillarMesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PillarMesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PillarMesh2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PillarMesh3;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> IronDoorSoundPlayOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> EscapeTriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> HandWheelOiledMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentLockPhase, BlueprintReadOnly, Category = "Lock", meta = (AllowPrivateAccess = "true"))
	E_LockPhase CurrentLockPhase;

	UFUNCTION()
	void OnRep_CurrentLockPhase();
	/*
	* Dynamic materials to make shake effect
	*/

	UFUNCTION()
	void InitMIDs();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ActivateShakeEffect();

	UFUNCTION()
	void DisableShakeEffect();

	UFUNCTION()
	void ToggleShakeEffect(bool bToggle);

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

	bool bIsChain;

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

	UPROPERTY(ReplicatedUsing = OnRep_DoorForceOpen)
	bool bDoorForceOpen;

	UFUNCTION()
	void OnRep_DoorForceOpen();

	void SetInteractionTraceable(bool bTraceable);

	float DoorAutoCloseSpeed;

	FTimerHandle DoorForceOpenTimerHandle;

	FTimerHandle ChainDropTimerHandle;

	/*
	Sound Managing
	*/
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

	// Escape
	UFUNCTION()
	void OnEscapeTriggerOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	TObjectPtr<UMaterialInterface> KeyIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	TObjectPtr<UMaterialInterface> HandleIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	TObjectPtr<UMaterialInterface> OilIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	FVector2D KeyIconSize = FVector2D(90.0f, 40.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	FVector2D HandleIconSize = FVector2D(80.0f, 80.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	FVector2D OilIconSize = FVector2D(60.0f, 70.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText UnlockText = FText::FromString(TEXT("Unlock"));

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText AttatchHandleText = FText::FromString(TEXT("Attatch"));

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText OilText = FText::FromString(TEXT("Oiling"));

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText CrankHandleText = FText::FromString(TEXT("Open"));

	TWeakObjectPtr<AActor> CachedChainUnlockInvestigator;
};
