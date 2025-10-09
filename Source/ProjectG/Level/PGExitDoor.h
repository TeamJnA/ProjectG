// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/InteractableActorInterface.h"

#include "PGExitDoor.generated.h"

class UBoxComponent;
class APGPlayerCharacter;

UCLASS()
class PROJECTG_API APGExitDoor : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGExitDoor();

	// IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	void SubtractLockStack() { LockStack--; }
	bool IsLocked() const;
	bool IsOpened() const;
	void ToggleDoor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EscapeMesh")
	TObjectPtr<UStaticMeshComponent> EscapeMesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EscapeMesh")
	TObjectPtr<UStaticMeshComponent> EscapeMesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trigger")
	TObjectPtr<UBoxComponent> EscapeTriggerVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(ReplicatedUsing = OnRep_LockStack)
	int32 LockStack;

	UPROPERTY(ReplicatedUsing = OnRep_DoorState)
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_LockStack();

	UFUNCTION()
	void OnRep_DoorState();

	UFUNCTION()
	void OnEscapeTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
