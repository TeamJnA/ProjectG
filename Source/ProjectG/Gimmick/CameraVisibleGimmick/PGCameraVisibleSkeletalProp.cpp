// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/CameraVisibleGimmick/PGCameraVisibleSkeletalProp.h"


APGCameraVisibleSkeletalProp::APGCameraVisibleSkeletalProp()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetVisibility(false);
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
}

void APGCameraVisibleSkeletalProp::ApplyCameraModeVisibility(bool bVisible)
{
	if (Mesh)
	{
		Mesh->SetVisibility(bVisible);
	}
}
