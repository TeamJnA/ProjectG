// Fill out your copyright notice in the Description page of Project Settings.


#include "PGGlobalLightManager.h"

#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
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

/*
* ���� ���� ��� LightComponent, EmissiveMaterial Ž��/����
* ���� ���� Ÿ�̸ӷ� �ֱ����� ���� ȿ��
*/
void APGGlobalLightManager::BeginPlay()
{
	Super::BeginPlay();

	ULevel* Level = GetWorld()->GetCurrentLevel();
	if (!Level || !Level->bIsVisible) 
	{
		return;
	}

	TArray<ULightComponent*> LightComps;
	for (const AActor* Actor : Level->Actors)
	{
		if (!Actor) 
		{
			continue;
		}

		Actor->GetComponents<ULightComponent>(LightComps);
		for (ULightComponent* Light : LightComps)
		{
			if (Light && Light->IsVisible())
			{
				ManagedLights.Add(Light);
			}
		}
	}
	InitializeEmissiveMIDs();

	UE_LOG(LogTemp, Log, TEXT("LightManager: %d lights found. | HasAuthority = %d"), ManagedLights.Num(), HasAuthority());

	if (HasAuthority())
	{		
		StartBlinkCycle();
	}
}

void APGGlobalLightManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
}

/*
* EmissiveMaterial Ž��/����
*/
void APGGlobalLightManager::InitializeEmissiveMIDs()
{
	ULevel* Level = GetWorld()->GetCurrentLevel();
	if (!Level || !Level->bIsVisible)
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;

	TArray<UMaterialInstanceDynamic*> MIDs;
	TArray<FName> ParamNames;
	TArray<float> InitialValues;

	for (AActor* Actor : Level->Actors)
	{
		if (!Actor)
		{
			continue;
		}

		Actor->GetComponents<UMeshComponent>(MeshComponents);
		for (UMeshComponent* MeshComp : MeshComponents)
		{
			int32 MaterialCount = MeshComp->GetNumMaterials();

			for (int32 i = 0; i < MaterialCount; ++i)
			{
				UMaterialInterface* Material = MeshComp->GetMaterial(i);
				if (!Material)
				{
					continue;
				}

				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Material, this);
				if (!MID) 
				{
					continue;
				}

				MeshComp->SetMaterial(i, MID);

				TArray<FMaterialParameterInfo> ParamInfos;
				TArray<FGuid> ParamIds;
				MID->GetAllScalarParameterInfo(ParamInfos, ParamIds);

				for (const FMaterialParameterInfo& Info : ParamInfos)
				{
					if (Info.Name == "EmissiveValue")
					{
						float defaultValue = 1.0f;
						MID->GetScalarParameterValue(Info.Name, defaultValue);

						MIDs.Add(MID);
						ParamNames.Add(Info.Name);
						InitialValues.Add(defaultValue);
						break;
					}
				}
			}
		}

		if (MIDs.Num() > 0)
		{
			EmissiveMIDs.Add(MIDs);
			EmissiveParamNames.Add(ParamNames);
			EmissiveInitialValues.Add(InitialValues);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LightManager: %d emissive mesh found. | HasAuthority = %d"), EmissiveMIDs.Num(), HasAuthority());
}

/*
* Blink ���� ȿ�� ����
* BlinkInterval(0.1��)���� Blink
*/
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

/*
* Blink ����
* Blink 2ȸ �ݺ� �� Blink Ÿ�̸Ӹ� ���� �� BlinkCycleInterval(10��) �� Blink ��ȣ��
* ToggleLight�� Light�� Ű�ų� ���� ���� -> ToggleLight 4ȸ �ݺ� == BlinkCycle 2ȸ
*/
void APGGlobalLightManager::Blink()
{
	Multicast_ToggleLight();
	BlinkCount++;

	if (BlinkCount >= MaxBlinksPerCycle * 2)
	{
		GetWorldTimerManager().ClearTimer(BlinkTimerHandle);

		FTimerHandle BlinkCycleTimerHandle;
		GetWorldTimerManager().SetTimer(
			BlinkCycleTimerHandle,
			this,
			&APGGlobalLightManager::StartBlinkCycle,
			BlinkCycleInterval,
			false
		);
	}
}

/*
* LightComponent, EmissiveMaterial ���
*/
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
		const TArray<FName>& ParamNames = EmissiveParamNames[i];
		const TArray<float>& InitialValues = EmissiveInitialValues[i];

		for (int32 j = 0; j < MIDs.Num(); ++j)
		{
			if (MIDs[j] && ParamNames.IsValidIndex(j) && InitialValues.IsValidIndex(j))
			{
				float newValue = bEmissiveOn ? InitialValues[j] : 0.0f;
				MIDs[j]->SetScalarParameterValue(ParamNames[j], newValue);
			}
		}
	}
}
