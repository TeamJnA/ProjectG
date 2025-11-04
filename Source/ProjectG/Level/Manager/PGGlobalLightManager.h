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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Blink();
	void StartBlinkCycle();
	void InitializeEmissiveMIDs();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleLight();

private:
	UPROPERTY()
	TArray<ULightComponent*> ManagedLights;
	TArray<TArray<UMaterialInstanceDynamic*>> EmissiveMIDs;
	TArray<TArray<FName>> EmissiveParamNames;
	TArray<TArray<float>> EmissiveInitialValues;
		
	FTimerHandle BlinkTimerHandle;

	float BlinkInterval = 0.1f;	
	float BlinkCycleInterval = 10.0f;

	int32 BlinkCount = 0;
	int32 MaxBlinksPerCycle = 2;
	bool bEmissiveOn = true;
};
