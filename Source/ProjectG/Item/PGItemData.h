// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "Type/CharacterTypes.h"
#include "PGItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	//Why using FText? https://dev.epicgames.com/documentation/ko-kr/unreal-engine/string-handling-in-unreal-engine
	UPROPERTY(EditDefaultsOnly)
	FText ItemName;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> ItemTexture2D;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMesh> ItemMesh;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> ItemAbility;

	UPROPERTY(EditDefaultsOnly)
	FTransform ItemSocketOffset;

	UPROPERTY(EditDefaultsOnly)
	EHandPoseType HandPoseType = EHandPoseType::Default;
};
