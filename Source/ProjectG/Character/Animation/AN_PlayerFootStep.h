// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_PlayerFootStep.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UAN_PlayerFootStep : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAN_PlayerFootStep();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	uint8 bIsPlayer : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName FootStepName;
};
