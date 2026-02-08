// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/PGChaosCacheManager.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Chaos/CacheCollection.h"

APGChaosCacheManager::APGChaosCacheManager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	GCMesh = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionMesh"));
	SetRootComponent(GCMesh);
}

void APGChaosCacheManager::PlayCached()
{
	SetActorTickEnabled(true);

	for (FObservedComponent& Observed : GetObservedComponents())
	{
		Observed.CacheName = PlayCacheName;
		Observed.bIsSimulating = false;
	}

	TriggerComponentByCache(PlayCacheName);

	if (DissolveCurve)
	{
		DissolveTimeline.PlayFromStart();
	}
}

void APGChaosCacheManager::UpdateDissolve(float Value)
{
	GCMesh->SetScalarParameterValueOnMaterials(DissolveParamName, Value);
}

void APGChaosCacheManager::OnDissolveFinished()
{
	SetActorTickEnabled(false);
}

void APGChaosCacheManager::BeginPlay()
{
	Super::BeginPlay();

	if (DissolveCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("UpdateDissolve"));
		DissolveTimeline.AddInterpFloat(DissolveCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("OnDissolveFinished"));
		DissolveTimeline.SetTimelineFinishedFunc(FinishedFunction);
	}
}

void APGChaosCacheManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DissolveTimeline.TickTimeline(DeltaTime);
}

void APGChaosCacheManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FindOrAddObservedComponent(GCMesh);
	if (GCMesh)
	{
		if (PGCacheCollection)
		{
			CacheCollection = PGCacheCollection;
		}
	}
}
