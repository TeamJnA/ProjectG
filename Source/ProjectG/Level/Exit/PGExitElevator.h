// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Components/TimelineComponent.h"
#include "PGExitElevator.generated.h"

class UCurveFloat;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGExitElevator : public APGExitPointBase
{
	GENERATED_BODY()
	
public:
	APGExitElevator();

	// IInteractableActorInterface~
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	virtual void InteractionFailed() override;
	// ~IInteractableActorInterface

	virtual bool Unlock() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ElevatorBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> InnerFenceBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> InnerFenceDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> OuterFenceBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> OuterFenceDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VisualMesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FusePanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimSequence>> FuseStatusAnim;

	int32 FuseState;

	///
	///		Door Close Timeline and functions
	/// 
	FTimeline DoorCloseTimeline;
};
