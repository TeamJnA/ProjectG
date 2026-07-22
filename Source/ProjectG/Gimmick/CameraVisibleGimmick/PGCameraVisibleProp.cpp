// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/CameraVisibleGimmick/PGCameraVisibleProp.h"
#include "Utils/PGCameraVisibleRegistry.h"
#include "EngineUtils.h"


APGCameraVisibleProp::APGCameraVisibleProp()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = Root;
}

void APGCameraVisibleProp::BeginPlay()
{
	Super::BeginPlay();
	
	ApplyCameraModeVisibility(false);

	if (UWorld* World = GetWorld())
	{
		if (UPGCameraVisibleRegistry* Registry = World->GetSubsystem<UPGCameraVisibleRegistry>())
		{
			Registry->RegisterProp(this);
		}
	}
}

void APGCameraVisibleProp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UPGCameraVisibleRegistry* Registry = World->GetSubsystem<UPGCameraVisibleRegistry>())
		{
			Registry->UnregisterProp(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void APGCameraVisibleProp::SetCameraModeVisible(bool bVisible)
{
	if (bCameraModeVisible == bVisible)
	{
		return;
	}

	bCameraModeVisible = bVisible;
	ApplyCameraModeVisibility(bVisible);
}
