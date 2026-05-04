// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SetCameraMeshOnHand.generated.h"

/**
 * 카메라 컴포넌트에서 아이템 메쉬 작업을 담당
 */
UCLASS()
class PROJECTG_API UAN_SetCameraMeshOnHand : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SetCameraMeshOnHand();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility", meta = (AllowPrivateAccess = "true"))
	bool bIsVisible;
};
