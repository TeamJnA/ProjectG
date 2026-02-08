// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/CacheManagerActor.h"
#include "Components/TimelineComponent.h"
#include "PGChaosCacheManager.generated.h"

class UGeometryCollectionComponent;
class UChaosCacheCollection;
class UCurveFloat;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGChaosCacheManager : public AChaosCacheManager
{
	GENERATED_BODY()

public:
	APGChaosCacheManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TObjectPtr<UGeometryCollectionComponent> GCMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TObjectPtr<UChaosCacheCollection> PGCacheCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FName PlayCacheName;

	UFUNCTION(BlueprintCallable)
	virtual void PlayCached();

	// Play Timeline part
	UPROPERTY(EditAnywhere, Category = "Destruction|Timeline")
	TObjectPtr<UCurveFloat> DissolveCurve;

	UPROPERTY(EditAnywhere, Category = "Destruction|Timeline")
	FName DissolveParamName = FName("Dissolve");

	UFUNCTION(BlueprintCallable)
	void UpdateDissolve(float Value);

	UFUNCTION()
	void OnDissolveFinished();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	FTimeline DissolveTimeline;
};
