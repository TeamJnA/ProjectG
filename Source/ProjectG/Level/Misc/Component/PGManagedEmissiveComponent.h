// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Interface/LightEffectInterface.h"
#include "PGManagedEmissiveComponent.generated.h"

USTRUCT()
struct FManagedMaterialInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MID;

	UPROPERTY()
	float OriginalValue;

	UPROPERTY()
	float TargetValue;

	FManagedMaterialInfo() : MID(nullptr), OriginalValue(0.0f), TargetValue(0.0f) {}
	FManagedMaterialInfo(UMaterialInstanceDynamic* InMID, float InOriginalValue) : MID(InMID), OriginalValue(InOriginalValue), TargetValue(InOriginalValue) {}
};

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UPGManagedEmissiveComponent : public USphereComponent, public ILightEffectInterface
{
	GENERATED_BODY()
	
public:
	UPGManagedEmissiveComponent();
	
	// ILightEffectInterface~
	virtual void FadeOut() override;
	virtual void FadeIn() override;
	// ~ILightEffectInterface

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EmissiveEffect")
	FName EmissiveParamName = FName("EmissiveValue");

	UPROPERTY(EditAnywhere, Category = "EmissiveEffect")
	float FadeUpdateRate = 0.1f;

	UPROPERTY(EditAnywhere, Category = "EmissiveEffect")
	float LightFadeSpeed = 3.0f;

private:
	UFUNCTION()
	void UpdateFade();

	void ManageFadeTimer();

	UPROPERTY()
	TArray<FManagedMaterialInfo> ManagedMaterials;

	FTimerHandle FadeTimerHandle;
};
