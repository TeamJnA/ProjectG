// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/Component/PGMannequinMeshComponent.h"


void UPGMannequinMeshComponent::OnRegister()
{
	Super::OnRegister();

	for (const FName& BoneName : HiddenBones)
	{
		HideBoneByName(BoneName, EPhysBodyOp::PBO_None);
	}
}
