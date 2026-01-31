// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Enemy/Charger/AI/E_PGChargerState.h"
#include "PGChargerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGChargerCharacter : public APGEnemyCharacterBase, public IPGAIExplorationInterface
{
	GENERATED_BODY()

public:
	APGChargerCharacter();

protected:
	virtual void BeginPlay() override;	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// IPGAIExplorationInterface~
	virtual float GetExplorationRadius() const override { return ExplorationRadius; }
	virtual float GetExplorationWaitTime() const override { return ExplorationWaitTime; }
	// ~IPGAIExplorationInterface
	virtual void GetActorEyesViewPoint(FVector& out_Location, FRotator& out_Rotation) const override;

	void SetHeadLookAtTarget(const FVector& NewTargetLocation);
	FVector GetHeadLookAtTarget() const;

	void RotateHeadYaw();
	FORCEINLINE float GetHeadYaw() const { return HeadYaw; }

	void SetCurrentState(E_PGChargerState NewState);
	FORCEINLINE E_PGChargerState GetCurrentState() const { return CurrentState; }

	void SetMovementSpeed(float NewSpeed);
	FORCEINLINE float GetPatrolSpeed() const { return PatrolSpeed; }
	FORCEINLINE float GetChargeSpeed() const { return ChargeSpeed; }
	FORCEINLINE float GetAdjustSpeed() const { return AdjustSpeed; }

	FORCEINLINE UAnimMontage* GetChargeMontage() const { return ChargeMontage; }
	FORCEINLINE UAnimMontage* GetKillMontage() const { return KillMontage; }

protected:
	virtual void OnTouchColliderOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	UFUNCTION()
	void OnRep_HeadYaw(float OldValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Montage")
	TObjectPtr<UAnimMontage> ChargeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Montage")
	TObjectPtr<UAnimMontage> KillMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attributes")
	TSubclassOf<UGameplayEffect> MovementSpeedEffectClass;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "AI|IK")
	FVector HeadLookAtLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Stats", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Stats", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Stats")
	float PatrolSpeed = 80.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Stats")
	float ChargeSpeed = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Stats")
	float AdjustSpeed = 400.0f;

	UPROPERTY(ReplicatedUsing = OnRep_HeadYaw, VisibleAnywhere, BlueprintReadOnly, Category = "AI|IK")
	float HeadYaw = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI|State")
	E_PGChargerState CurrentState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName HeadRotate180Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName HeadRotateStareName;
};
