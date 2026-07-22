// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/CameraVisibleGimmick/PGCameraVisiblePaintingProp.h"


APGCameraVisiblePaintingProp::APGCameraVisiblePaintingProp()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void APGCameraVisiblePaintingProp::BeginPlay()
{
	if (Mesh && Mesh->GetMaterials().IsValidIndex(PaintingMaterialIndex))
	{
		PaintingMID = Mesh->CreateAndSetMaterialInstanceDynamic(PaintingMaterialIndex);
	}

	Super::BeginPlay();
}

void APGCameraVisiblePaintingProp::ApplyCameraModeVisibility(bool bVisible)
{
	if (PaintingMID)
	{
		PaintingMID->SetScalarParameterValue(CameraModeParamName, bVisible ? 1.0f : 0.0f);
	}
}
