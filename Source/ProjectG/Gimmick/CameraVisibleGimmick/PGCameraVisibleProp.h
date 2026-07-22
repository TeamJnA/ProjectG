// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGCameraVisibleProp.generated.h"

UCLASS(Abstract)
class PROJECTG_API APGCameraVisibleProp : public AActor
{
	GENERATED_BODY()
	
public:	
	APGCameraVisibleProp();
	void SetCameraModeVisible(bool bVisible);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyCameraModeVisibility(bool bVisible) {}

	UPROPERTY(VisibleAnywhere, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	bool bCameraModeVisible = false;
};
