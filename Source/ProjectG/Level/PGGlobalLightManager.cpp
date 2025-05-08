// Fill out your copyright notice in the Description page of Project Settings.


#include "PGGlobalLightManager.h"
#include "Components/LightComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"

// Sets default values
APGGlobalLightManager::APGGlobalLightManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APGGlobalLightManager::BeginPlay()
{
	Super::BeginPlay();

	ULevel* level = GetWorld()->GetCurrentLevel();
	if(!level || !level->bIsVisible) return;

	for (AActor* Actor : level->Actors)
	{
		if (!Actor) continue;

		TArray<ULightComponent*> LightComps;
		Actor->GetComponents<ULightComponent>(LightComps);

		for (ULightComponent* Light : LightComps)
		{
			if (Light && Light->IsVisible())
			{
				ManagedLights.Add(Light);
				InitialIntensities.Add(Light->Intensity);
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		LightFadeTimer,
		this,
		&APGGlobalLightManager::UpdateLightIntensity,
		UpdateInterval,
		true
	);
	
	UE_LOG(LogTemp, Log, TEXT("LightManager: %d lights found."), ManagedLights.Num());

}

void APGGlobalLightManager::UpdateLightIntensity()
{
	ElapsedTime += UpdateInterval;
	float Alpha = FMath::Clamp(ElapsedTime / MaxTime, 0.0f, 1.0f);

	for (int32 i = 0; i < ManagedLights.Num(); ++i)
	{
		if (ManagedLights[i])
		{
			float NewIntensity = FMath::Lerp(InitialIntensities[i], 0.05f, Alpha);
			ManagedLights[i]->SetIntensity(NewIntensity);
			
			// UE_LOG(LogTemp, Log, TEXT("LightManager : working"), ManagedLights.Num());
		}
	}

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LightFadeTimer);
		UE_LOG(LogTemp, Log, TEXT("LightManger : Light Fade Complete"));
	}
}
