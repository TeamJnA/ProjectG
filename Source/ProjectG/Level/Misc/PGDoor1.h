// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/InteractableActorInterface.h"

#include "PGDoor1.generated.h"

UCLASS()
class PROJECTG_API APGDoor1 : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGDoor1();
	static void SpawnDoor(UWorld* World, const FTransform& Transform, const FActorSpawnParameters& SpawnParams, bool _bIsLocked);

	//IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	virtual void InteractionFailed() override;
	//IInteractableActorInterface end
	
	void ToggleDoor(AActor* InteractInvestigator);

	bool IsOpen() const { return bIsOpen; }
	bool IsLocked() const { return bIsLocked; }
	void Lock() { bIsLocked = true; OnRep_LockState(); }
	void UnLock();

	void PlayDoorSound(const FName& SoundName);

	void TEST_OpenDoorByAI(AActor* InteractInvestigator);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<USceneComponent> DoorHinge;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(ReplicatedUsing = OnRep_DesiredTransform)
	FTransform DesiredTransform;

	FTransform ClosedTransform;
	FTransform OpenedTransform_A;
	FTransform OpenedTransform_B;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DoorState", meta = (AllowPrivateAccess = "true"), Replicated)
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_LockState)
	bool bIsLocked = false;

	UFUNCTION()
	void OnRep_DesiredTransform();

	UFUNCTION()
	void OnRep_LockState();

	void SetDoorState(bool _bIsOpen, AActor* InteractInvestigator);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorOpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorCloseSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName DoorUnlockSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName LockedDoorSound;

	// Door Shake
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDDoor;

	FTimerHandle ShakeEffectTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect", meta = (AllowPrivateAccess = "true"))
	FName ShakeParameterName;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ActivateShakeEffect();

	UFUNCTION()
	void DisableShakeEffect();

	UFUNCTION()
	void ToggleShakeEffect(bool bToggle);
};
