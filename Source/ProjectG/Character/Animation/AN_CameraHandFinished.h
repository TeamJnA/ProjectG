// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_CameraHandFinished.generated.h"

/**
 * 카메라 컴포넌트에 접근하여, 카메라 들어올리는 애니메이션이 끝남을 알림.
 */
UCLASS()
class PROJECTG_API UAN_CameraHandFinished : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
