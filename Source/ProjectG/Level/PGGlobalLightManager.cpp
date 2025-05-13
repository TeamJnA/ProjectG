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
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void APGGlobalLightManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("LightManager BeginPlay: %s | HasAuthority = %d"), *GetName(), HasAuthority());

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("LightManager on Server"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LightManager on Client"));
	}

	// Get all light on level
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
	
	UE_LOG(LogTemp, Log, TEXT("LightManager: %d lights found. | HasAuthority = %d"), ManagedLights.Num(), HasAuthority());

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			LightFadeTimer,
			this,
			&APGGlobalLightManager::UpdateLightIntensity,
			UpdateInterval,
			true
		);
	}
}

// Update alpha on server
void APGGlobalLightManager::UpdateLightIntensity()
{
	ElapsedTime += UpdateInterval;
	float Alpha = FMath::Clamp(ElapsedTime / MaxTime, 0.0f, 1.0f);

	// Update light
	Multicast_UpdateLightIntensity(Alpha);

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LightFadeTimer);
		UE_LOG(LogTemp, Log, TEXT("LightManger : Light Fade Complete"));
	}
}

void APGGlobalLightManager::Multicast_UpdateLightIntensity_Implementation(float Alpha)
{
	//UE_LOG(LogTemp, Warning, TEXT("UpdateLightIntensity | HasAuthority = %d"), HasAuthority());

	for (int32 i = 0; i < ManagedLights.Num(); ++i)
	{
		if (ManagedLights[i])
		{
			float newIntensity = FMath::Lerp(InitialIntensities[i], 0.05f, Alpha);
			ManagedLights[i]->SetIntensity(newIntensity);
		}
	}
}
