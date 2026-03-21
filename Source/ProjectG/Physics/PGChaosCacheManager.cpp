// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/PGChaosCacheManager.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Chaos/CacheCollection.h"
#include "Physics/PhysicsInterfaceCore.h" 

APGChaosCacheManager::APGChaosCacheManager()
{
	bReplicates = true;

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	GCMesh = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionMesh"));
	SetRootComponent(GCMesh);
}

void APGChaosCacheManager::PlayCached()
{

	for (FObservedComponent& Observed : GetObservedComponents())
	{
		Observed.CacheName = PlayCacheName;
		Observed.bIsSimulating = false;
	}

	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* Scene = World->GetPhysicsScene())
		{
			Scene->WaitPhysScenes();
		}
	}

	BeginEvaluate();

	SetActorHiddenInGame(false);

	TriggerComponentByCache(PlayCacheName);
	
	if (DissolveCurve)
	{
		SetActorTickEnabled(true);
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

	// Destroy this actor
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[APGChaosCacheManager] Destroyed after broken"));
		SetLifeSpan(2.0f);
	}
}

void APGChaosCacheManager::Multicast_CleanupGeometyCollection_Implementation()
{
	if (GCMesh)
	{
		GCMesh->SetSimulatePhysics(false);

		if (GCMesh->IsPhysicsStateCreated())
		{
			GCMesh->DestroyPhysicsState();
		}
		GCMesh->SetRestCollection(nullptr);
	}
}

void APGChaosCacheManager::BeginPlay()
{
	using namespace Chaos;

	AActor::BeginPlay();

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
