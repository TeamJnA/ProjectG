// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "Type/CharacterTypes.h"
#include "PGItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemActionGuide
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FText ActionText;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> KeyIcon;

	UPROPERTY(EditDefaultsOnly)
	FVector2D KeyIconSize = FVector2D(15.0f, 15.0f);
};

/** 아이템 Drop 시 취할 수 있는 자세.(Pitch 조절해야하는 경우가 있고 Roll 조절해야하는 경우가 있음) */
USTRUCT(BlueprintType)
struct FItemDropPose
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float Pitch = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float Roll = 0.0f;
};

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
	TArray<FItemActionGuide> ActionGuides;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TArray<FItemDropPose> DropPoses;

	UPROPERTY(EditDefaultsOnly)
	EHandPoseType HandPoseType = EHandPoseType::Default;

	FRotator GetRandomDropRotation() const;
};
