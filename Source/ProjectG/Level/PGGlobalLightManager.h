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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightControl")
	float MaxTime = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightControl")
	float UpdateInterval = 0.2f;

	float ElapsedTime = 0.0f;

	FTimerHandle LightFadeTimer;

	void UpdateLightIntensity();

};
