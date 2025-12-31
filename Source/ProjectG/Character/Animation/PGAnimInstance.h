// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Type/CharacterTypes.h"
#include "Interface/CharacterAnimationInterface.h"
#include "PGAnimInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnim, Log, All);
/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAnimInstance : public UAnimInstance, public ICharacterAnimationInterface
{
	GENERATED_BODY()

public:
	UPGAnimInstance();

	// ICharacterAnimationInterface~
	virtual void SetHandPose(EHandPoseType NewHandPoseType) override;
	// ~ICharacterAnimationInterface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HandPose", meta = (AllowPrivateAccess = "true"))
	TArray<float> HandPoseWeights;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HandPose", meta = (AllowPrivateAccess = "true"))
	int32 HandPoseCount;
};
