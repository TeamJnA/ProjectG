// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Components/TimelineComponent.h"
#include "PGExitElevator.generated.h"

class UBoxComponent;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGExitElevator : public APGExitPointBase
{
	GENERATED_BODY()
	
public:
	APGExitElevator(); 

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// IInteractableActorInterface~
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	virtual void InteractionFailed() override;
	// ~IInteractableActorInterface

	virtual bool Unlock() override;

private:

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> FusePanelCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ElevatorBodyCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimSequence>> FuseStatusAnim;

	UPROPERTY(ReplicatedUsing = OnRep_FuseState, BlueprintReadOnly, Category = "Fuse", meta = (AllowPrivateAccess = "true"))
	int32 FuseState;

	UFUNCTION()
	void OnRep_FuseState();

	void ExecuteEscapeSequence();

	void EscapePlayers();

	FTimerHandle EscapeTimerHandle;

	///
	///		Door Close Timeline and functions
	/// 
	UFUNCTION()
	void DoorCloseProgress(float Value);

	UFUNCTION()
	void DoorCloseFinished();

	UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> DoorCloseCurveFloat;

	FTimeline DoorCloseTimeline;

	bool bInnerDoorClosed;

	FVector BaseInnerFenceLocation;

	FVector BaseOuterFenceLocation;

	// 문이 닫히는 X좌표를 블루프린트에서 수정 가능하게.
	UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	float InnerFenceClosedX = 142.0f;

	UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	float OuterFenceClosedX = 126.0f;

	///
	/// Elevator Descent Timeline and functions
	/// 
	UFUNCTION()
	void ElevatorDescentProgress(float Value);

	UFUNCTION()
	void ElevatorDescentFinished();

	UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> ElevatorDescentCurveFloat;

	FTimeline ElevatorDescentTimeline;

	UPROPERTY(EditAnywhere, Category = "Timeline", meta = (AllowPrivateAccess = "true"))
	float ElevatorDescentTargetZ = -1200.0f;
};
