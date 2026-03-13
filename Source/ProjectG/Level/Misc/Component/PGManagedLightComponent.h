// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Interface/LightEffectInterface.h"
#include "PGManagedLightComponent.generated.h"

class ULightComponent;

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTG_API UPGManagedLightComponent : public USphereComponent, public ILightEffectInterface
{
	GENERATED_BODY()

public:
	UPGManagedLightComponent();
	
	// ILightEffectInterface~
	virtual void FadeOut() override;
	virtual void FadeIn() override;
	virtual void PowerOff() override;
	virtual bool IsPermanentOff() const override { return bPermanentOff; }
	// ~ILightEffectInterface

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "LightEffect")
	float FadeUpdateRate = 0.1f;

	UPROPERTY(EditAnywhere, Category = "LightEffect")
	float LightFadeSpeed = 3.0f;

private:
	UFUNCTION()
	void UpdateFade();

	void ManageFadeTimer();

	void PowerOffSequence();

	UPROPERTY()
	TObjectPtr<ULightComponent> TargetLight;

	FTimerHandle FadeTimerHandle;
	FTimerHandle PowerOffTimerHandle;

	float OriginalIntensity = 0.0f;
	float TargetIntensity = 0.0f;

	int32 PowerOffStep = 0;

	bool bPermanentOff = false;
};
