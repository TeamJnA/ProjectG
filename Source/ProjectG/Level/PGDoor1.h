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
	//static void SpawnDoor(UWorld* World, const FTransform& Transform, const FActorSpawnParameters& SpawnParams, bool _bIsLocked);

	//IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	void HighlightOn() const override;
	void HighlightOff() const override;
	//IInteractableActorInterface end
	
	void ToggleDoor();

	UFUNCTION(Server, Reliable)
	void Server_ToggleDoor();

	bool IsOpen() const { return bIsOpen; }
	bool IsLocked() const { return bIsLocked; }
	void Lock() { bIsLocked = true; OnRep_LockState(); }
	void UnLock() { bIsLocked = false; OnRep_LockState(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WallMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(ReplicatedUsing = OnRep_DoorState)
	bool bIsOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_LockState)
	bool bIsLocked = false;

	UFUNCTION()
	void OnRep_DoorState();

	UFUNCTION()
	void OnRep_LockState();

	void SetDoorState(bool _bIsOpen);

	UFUNCTION(NetMulticast, Reliable)
	void NM_SetDoorState(bool _bIsOpen);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void PostNetInit() override;
	virtual void PostNetReceive() override;
};
