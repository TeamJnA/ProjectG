// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGCameraVisibleRegistry.generated.h"

class APGCameraVisibleProp;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGCameraVisibleRegistry : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void RegisterProp(APGCameraVisibleProp* Prop);
	void UnregisterProp(APGCameraVisibleProp* Prop);

	void SetCameraModeVisible(bool bVisible);

	FORCEINLINE bool IsCameraModeVisible() const { return bCameraModeVisible; }

private:
	TArray<TWeakObjectPtr<APGCameraVisibleProp>> Props;
	bool bCameraModeVisible = false;
};
