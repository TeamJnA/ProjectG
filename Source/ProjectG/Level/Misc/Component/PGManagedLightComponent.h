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
	virtual void PermanentOff() override;
	virtual bool IsPermanentOff() const override { return bPermanentOff; }
	// ~ILightEffectInterface

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "LightEffect")
	float FadeUpdateRate = 0.1f;

	UPROPERTY(EditAnywhere, Category = "LightEffect")
	float LightFadeSpeed = 3.0f;

private:
	FTimerHandle FadeTimerHandle;

	UPROPERTY()
	TObjectPtr<ULightComponent> TargetLight;

	float OriginalIntensity = 0.0f;
	float TargetIntensity = 0.0f;

	bool bPermanentOff = false;

	UFUNCTION()
	void UpdateFade();

	void ManageFadeTimer();
};
