// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PGChargerAnimInstance.generated.h"

class APGChargerCharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGChargerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void ApplyRotationOffset(const float Offset) { CurrentHeadYaw -= Offset; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<APGChargerCharacter> ChargerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector LookAtLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
	float CurrentHeadYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
	float InterpSpeed = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	bool bUseLookAtTarget;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsAdjusting;
};
