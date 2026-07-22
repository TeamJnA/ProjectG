// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/CameraVisibleGimmick/PGCameraVisibleProp.h"
#include "PGCameraVisibleSkeletalProp.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGCameraVisibleSkeletalProp : public APGCameraVisibleProp
{
	GENERATED_BODY()

public:
	APGCameraVisibleSkeletalProp();

protected:
	virtual void ApplyCameraModeVisibility(bool bVisible) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> Mesh;	
};
