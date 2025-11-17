// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_PGPredictThrowableTrajectory.generated.h"

/**
 * 
 */

class USplineComponent;
class USplineMeshComponent;

UCLASS()
class PROJECTG_API UAT_PGPredictThrowableTrajectory : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UAT_PGPredictThrowableTrajectory(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Task", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_PGPredictThrowableTrajectory* DrawTrajectory(UGameplayAbility* OwningAbility, float InitialSpeed);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	UPROPERTY()
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshComponents;

	float InitialSpeed;

	TObjectPtr<UStaticMesh> CachedSplineMesh;

	TObjectPtr<UMaterialInterface> CachedSplineMaterial;

	int32 SplineMeshesCount;
};
