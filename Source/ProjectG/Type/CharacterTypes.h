#pragma once

#include "CoreMinimal.h"

#include "GameFramework/OnlineReplStructs.h"

#include "CharacterTypes.generated.h"

/**
 * Custom enum to demonstrate how to expose enums to Blueprints.
 */

UENUM(BlueprintType)
enum class EHandActionMontageType : uint8
{
	Pick,
	Change,
	Drop,
	CameraOn,
	CameraOff
};

UENUM(BlueprintType)
enum class EGameTeam : uint8
{
	Player	UMETA(DisplayName = "Player"),
	AI		UMETA(DisplayName = "AI")
};

UENUM(BlueprintType)
enum class EHandPoseType : uint8
{
	Default,
	Grab,
	Pinch,
	Count UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Blind,
	Charger,
	Ghost,
	Count UMETA(Hidden)
};

USTRUCT()
struct FBloodTextureEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditDefaultsOnly)
	float UVTiling = 3.0f;
};

USTRUCT(BlueprintType)
struct FBloodTextureSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TArray<FBloodTextureEntry> BloodTextures;
};

USTRUCT()
struct FPGRagdollSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize100 RootLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator RootRotation = FRotator::ZeroRotator;

	// 루트 기준 각 바디의 상대 트랜스폼 (Bodies 배열 순서)
	UPROPERTY()
	TArray<FTransform> BodyTransforms;

	UPROPERTY()
	bool bValid = false;
};
