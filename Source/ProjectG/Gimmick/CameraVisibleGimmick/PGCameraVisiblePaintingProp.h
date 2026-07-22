// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/CameraVisibleGimmick/PGCameraVisibleProp.h"
#include "PGCameraVisiblePaintingProp.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGCameraVisiblePaintingProp : public APGCameraVisibleProp
{
	GENERATED_BODY()
	
public:
	APGCameraVisiblePaintingProp();

protected:
	virtual void BeginPlay() override;
	virtual void ApplyCameraModeVisibility(bool bVisible) override;

	UPROPERTY(EditDefaultsOnly, Category = "Painting")
	FName CameraModeParamName = FName("CameraModeVisible");

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PaintingMID;

	UPROPERTY(EditDefaultsOnly, Category = "Painting")
	int32 PaintingMaterialIndex = 1;
};
