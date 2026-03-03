// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/PGAnimInstance.h"
#include "Character/PGPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogAnim);

UPGAnimInstance::UPGAnimInstance()
{
	HandPoseCount = static_cast<int32>(EHandPoseType::Count);

	HandPoseWeights.SetNum(HandPoseCount);
}

void UPGAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(TryGetPawnOwner()))
	{
		const float TargetScale = Player->IsTalking() ? TalkingHeadScale : 1.0f;
		HeadBoneScale = FMath::FInterpTo(HeadBoneScale, TargetScale, DeltaSeconds, HeadScaleLerpSpeed);
	}
}

void UPGAnimInstance::SetHandPose(EHandPoseType NewHandPoseType)
{
	const int32 PoseIndex = static_cast<int32>(NewHandPoseType);
	
	check(HandPoseWeights.IsValidIndex(PoseIndex));

	// EHandPoseType [0] == Default.
	// HandPoseType이 Default일 때는 커브값들이 모두 0이어야 한다.
	// 따라서 for문을 1부터 진행.
	for (int32 i = 1; i < HandPoseCount; i++)
	{
		HandPoseWeights[i] = (i == PoseIndex) ? 1.0f : 0.0f;
	}
}
