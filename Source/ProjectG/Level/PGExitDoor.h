// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/InteractableActorInterface.h"

#include "PGExitDoor.generated.h"

UCLASS()
class PROJECTG_API APGExitDoor : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGExitDoor();

	// IInteractableActorInterface~
	TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	void HighlightOn() const override;
	void HighlightOff() const override;
	// ~IInteractableActorInterface

	void SubtractLockStack() { LockStack--; }
	bool IsLocked() const { return (LockStack > 0); }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(Replicated)
	int32 LockStack;

	UPROPERTY(ReplicatedUsing = OnRep_DoorState)
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_DoorState();


};
