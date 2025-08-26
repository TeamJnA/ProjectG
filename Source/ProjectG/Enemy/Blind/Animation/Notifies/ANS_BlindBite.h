// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_BlindBite.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnemyANS, Log, All);

//�� ��Ƽ���̴� bite collider �� animation�� ���缭 enable/disable �ϴ� ��.
/**
 * 
 */
UCLASS()
class PROJECTG_API UANS_BlindBite : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	//virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};
