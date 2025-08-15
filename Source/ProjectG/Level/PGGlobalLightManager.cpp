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

	// setup light manager
	// Get all light on level
	ULevel* level = GetWorld()->GetCurrentLevel();
	if(!level || !level->bIsVisible) return;

	for (AActor* actor : level->Actors)
	{
		if (!actor) continue;

		TArray<ULightComponent*> lightComps;
		actor->GetComponents<ULightComponent>(lightComps);

		for (ULightComponent* light : lightComps)
		{
			if (light && light->IsVisible())
			{
				ManagedLights.Add(light);
				InitialIntensities.Add(light->Intensity);
			}
		}

	}
	InitializeEmissiveMIDs();

	UE_LOG(LogTemp, Log, TEXT("LightManager: %d lights found. | HasAuthority = %d"), ManagedLights.Num(), HasAuthority());


	// timer only on server
	if (HasAuthority())
	{		
		StartBlinkCycle();

		/*
		GetWorld()->GetTimerManager().SetTimer(
			LightFadeTimerHandle,
			this,
			&APGGlobalLightManager::UpdateLightIntensity,
			LightFadeUpdateInterval,
			true
		);
		*/
	}
}

// Update alpha on server
void APGGlobalLightManager::UpdateLightIntensity()
{
	ElapsedTime += LightFadeUpdateInterval;
	float Alpha = FMath::Clamp(ElapsedTime / MaxTime, 0.0f, 1.0f);

	// Update light
	Multicast_UpdateLightIntensity(Alpha);

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LightFadeTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("LightManger : Light Fade Complete"));
	}
}

void APGGlobalLightManager::StartBlinkCycle()
{
	BlinkCount = 0;

	GetWorld()->GetTimerManager().SetTimer(
		BlinkTimerHandle,
		this,
		&APGGlobalLightManager::Blink,
		BlinkInterval,
		true
	);
}

void APGGlobalLightManager::Blink()
{
	// light on/off
	Multicast_ToggleLight();
	BlinkCount++;

	// blinkcount on/off => MaxBlinksPerCycle * 2
	if (BlinkCount >= MaxBlinksPerCycle * 2)
	{
		GetWorldTimerManager().ClearTimer(BlinkTimerHandle);

		GetWorldTimerManager().SetTimer(
			BlinkCycleTimerHandle,
			this,
			&APGGlobalLightManager::StartBlinkCycle,
			BlinkCycleInterval,
			false
		);
	}
}

void APGGlobalLightManager::InitializeEmissiveMIDs()
{
	EmissiveMIDs.Empty();
	EmissiveParamNames.Empty();
	EmissiveInitialValues.Empty();

	ULevel* level = GetWorld()->GetCurrentLevel();
	if (!level || !level->bIsVisible) return;

	for (AActor* actor : level->Actors)
	{
		if (!actor) continue;

		TArray<UMeshComponent*> meshComponents;
		actor->GetComponents<UMeshComponent>(meshComponents);

		TArray<UMaterialInstanceDynamic*> MIDs;
		TArray<FName> paramNames;
		TArray<float> initialValues;

		for (UMeshComponent* meshComp : meshComponents)
		{
			int32 materialCount = meshComp->GetNumMaterials();

			for (int32 i = 0; i < materialCount; ++i)
			{
				UMaterialInterface* material = meshComp->GetMaterial(i);
				if (!material) continue;

				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(material, this);
				if (!MID) continue;

				meshComp->SetMaterial(i, MID);

				TArray<FMaterialParameterInfo> paramInfos;
				TArray<FGuid> paramIds;
				MID->GetAllScalarParameterInfo(paramInfos, paramIds);

				for (const FMaterialParameterInfo& info : paramInfos)
				{
					const FString paramName = info.Name.ToString().ToLower();

					//if (paramName == "light intensity" || paramName == "intensity" || paramName == "brightness")
					if (paramName == "emissivevalue")
					{
						float defaultValue = 1.0f;

						MID->GetScalarParameterValue(info.Name, defaultValue);

						MIDs.Add(MID);
						paramNames.Add(info.Name);
						initialValues.Add(defaultValue);
						break;
					}
				}
			}
		}

		if (MIDs.Num() > 0)
		{
			EmissiveMIDs.Add(MIDs);
			EmissiveParamNames.Add(paramNames);
			EmissiveInitialValues.Add(initialValues);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LightManager: %d emissive mesh found. | HasAuthority = %d"), EmissiveMIDs.Num(), HasAuthority());

}

void APGGlobalLightManager::Multicast_ToggleLight_Implementation()
{

	for (int32 i = 0; i < ManagedLights.Num(); ++i)
	{
		if (ManagedLights[i])
		{
			ManagedLights[i]->ToggleVisibility();
		}
	}

	bEmissiveOn = !bEmissiveOn;

	for (int32 i = 0; i < EmissiveMIDs.Num(); ++i)
	{
		const TArray<UMaterialInstanceDynamic*>& MIDs = EmissiveMIDs[i];
		const TArray<FName>& paramNames = EmissiveParamNames[i];
		const TArray<float>& initialValues = EmissiveInitialValues[i];

		for (int32 j = 0; j < MIDs.Num(); ++j)
		{
			if (MIDs[j] && paramNames.IsValidIndex(j) && initialValues.IsValidIndex(j))
			{
				float newValue = bEmissiveOn ? initialValues[j] : 0.0f;
				MIDs[j]->SetScalarParameterValue(paramNames[j], newValue);
			}
		}
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

	for (int32 i = 0; i < EmissiveMIDs.Num(); ++i)
	{
		const TArray<UMaterialInstanceDynamic*>& MIDs = EmissiveMIDs[i];
		const TArray<FName>& paramNames = EmissiveParamNames[i];
		const TArray<float>& initialValues = EmissiveInitialValues[i];

		for (int32 j = 0; j < MIDs.Num(); ++j)
		{
			if (MIDs[j] && paramNames.IsValidIndex(j) && initialValues.IsValidIndex(j))
			{
				float newIntensity = FMath::Lerp(initialValues[j], 0.0f, Alpha);
				MIDs[j]->SetScalarParameterValue(paramNames[j], newIntensity);
			}
		}
	}
}
