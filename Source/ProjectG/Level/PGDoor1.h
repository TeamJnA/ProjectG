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
	//IInteractableActorInterface end
	
	void ToggleDoor();

	bool IsOpen() const { return bIsOpen; }
	bool IsLocked() const { return bIsLocked; }
	void Lock() { bIsLocked = true; OnRep_LockState(); }
	void UnLock() { bIsLocked = false; OnRep_LockState(); }

	void TEST_OpenDoorByAI();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WallMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DoorState", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_DoorState)
	bool bIsOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_LockState)
	bool bIsLocked = false;

	UFUNCTION()
	void OnRep_DoorState();

	UFUNCTION()
	void OnRep_LockState();

	void SetDoorState(bool _bIsOpen);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
};
