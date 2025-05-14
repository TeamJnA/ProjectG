// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGGlobalLightManager.generated.h"

UCLASS()
class PROJECTG_API APGGlobalLightManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGGlobalLightManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<ULightComponent*> ManagedLights;
	TArray<float> InitialIntensities;
	TArray<TArray<UMaterialInstanceDynamic*>> EmissiveMIDs;
	TArray<TArray<FName>> EmissiveParamNames;
	TArray<TArray<float>> EmissiveInitialValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightControl")
	float MaxTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightControl")
	float LightFadeUpdateInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightControl")
	float BlinkInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightControl")
	float BlinkCycleInterval = 10.0f;

	float ElapsedTime = 0.0f;

	FTimerHandle LightFadeTimerHandle;
	FTimerHandle BlinkTimerHandle;
	FTimerHandle BlinkCycleTimerHandle;

	void UpdateLightIntensity();
	void StartBlinkCycle();
	void Blink();
	void InitializeEmissiveMIDs();

	int32 BlinkCount;
	int32 MaxBlinksPerCycle = 2;

	bool bEmissiveOn = true;
	float EmissiveOnValue = 5.0f;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleLight();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateLightIntensity(float Alpha);
};
