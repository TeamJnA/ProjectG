// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PGItemData.h"


FRotator UPGItemData::GetRandomDropRotation() const
{
	FRotator Result(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	if (DropPoses.Num() > 0)
	{
		const FItemDropPose& Pose = DropPoses[FMath::RandRange(0, DropPoses.Num() - 1)];
		Result.Pitch = Pose.Pitch;
		Result.Roll = Pose.Roll;
	}

	return Result;
}
